#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <string>

#include "interaction.h"
#include "wrappers.h"
#include "validation.h"
#include "udpClient.h"

extern int MAX_LEN;

//int UDP_PORT = 23101;
//#include <ctime>

using namespace std;

struct UDPPacket {
    vector<char> buffer= vector<char>(MAX_LEN);
    struct sockaddr_in client_addr;
    socklen_t addr_len;
};

// Функция запуска клиента (UDP)
void startUdpClient(const char *ip, int port) {
    int client_fd = Socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    Inet_pton(AF_INET, ip, (void*)&server_addr.sin_addr);

    socklen_t addr_len = sizeof(server_addr);
    char buffer[MAX_LEN];
    pair<int, vector<int>> res;
    pair<int, int> startEnd;

    bool activity_client = true;
    int inputTp;
    vector<vector<int>> graph;

    while(activity_client) {
        bzero(buffer, MAX_LEN);
        inputTp = inputType(buffer, MAX_LEN, activity_client);

        // Проверка на выход
        if (strcmp(buffer, "exit") == 0) {
            sendto(client_fd, buffer, strlen(buffer), 0,
                   (struct sockaddr*)&server_addr, addr_len);
            printf("Клиент отключен\n");
            break;
        }

        graph.clear();
        if (!inputTp) {
            inputGraphUDP(graph);
        } else {
            readGraphUDP(graph);
        }

        bzero(buffer, MAX_LEN);
        startEnd = startEndNode(buffer);

        // Проверяем на минимальное и максимальное кол-во вершин и рёбер
        if (!numberVertexes(graph, 1)) continue;
        // Проверяем на вхождение вершин в граф
        if (!isVertexesInBorder(startEnd.first, startEnd.second, graph, 1)) continue;
        // Проверка на связность графа (вершины должны быть соединены друг с другом, если одна соединена с другой)
        if (!connectivityVertexes(graph, 1)) continue;

        // Отправляем команду "graph" серверу
        strcpy(buffer, "graph");
        sendto(client_fd, buffer, strlen(buffer), 0,
               (struct sockaddr*)&server_addr, addr_len);

        // Блок ожидания подтверждения связи с сервером
        bool received_response = false;
        int attempts = 0;
        const int max_attempts = 3;
        const int timeout_seconds = 3;

        while (attempts < max_attempts && !received_response) {
            // Устанавливаем таймаут на сокет
            struct timeval tv;
            tv.tv_sec = timeout_seconds;
            tv.tv_usec = 0;
            setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

            bzero(buffer, MAX_LEN);
            ssize_t recv_len = recvfrom(client_fd, buffer, MAX_LEN, 0,
                                        (struct sockaddr*)&server_addr, &addr_len);

            if (recv_len > 0) {
                received_response = true;
            } else {
                attempts++;
                if (attempts < max_attempts) {
                    cout << "Не получен ответ от сервера. Повторная попытка "
                         << attempts << " из " << max_attempts << "..." << endl;

                    // Повторно отправляем команду перед следующей попыткой
                    sendto(client_fd, "graph", strlen("graph"), 0,
                           (struct sockaddr*)&server_addr, addr_len);
                }
            }
        }

        // Сбрасываем таймаут после попыток
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        if (!received_response) {
            cout << "Проблема ожидания: сервер не отвечает после "
                 << max_attempts << " попыток." << endl;
            continue; // Переходим к следующей итерации цикла
        }
        // Блок ожидания подтверждения связи с сервером заканчивается

        // В случае потери данных начинаем процесс задания графа с начала
        if (strcmp(buffer, "OK") != 0) {
            printf("Ошибка связи с сервером\n");
            continue;
        }

        // Отправляем граф
        sendGraphUDP(client_fd, server_addr, addr_len, graph, startEnd.first, startEnd.second);

        // Получаем результат
        bzero(buffer, MAX_LEN);
        ssize_t recv_len = recvfrom(client_fd, buffer, MAX_LEN, 0,
                                    (struct sockaddr*)&server_addr, &addr_len);

        if (recv_len > 0) {
            // Парсим результат (строчный вид преобразовываем в вектор)
            size_t i = 0;
            string dig;
            res.second.clear();

            for (i = 0; i < MAX_LEN; ++i) {
                if (buffer[i] == '/') {
                    break;
                } else if (buffer[i] == ',') {
                    res.first = stoll(dig);
                    dig = "";
                    continue;
                }
                dig += buffer[i];
            }

            i++;
            dig = "";
            for (; i < MAX_LEN && buffer[i] != '\0'; ++i) {
                if (buffer[i] == ',') {
                    if (!dig.empty()) {
                        res.second.push_back(stoi(dig));
                        dig = "";
                    }
                    continue;
                }
                dig += buffer[i];
            }

            if (!dig.empty()) {
                res.second.push_back(stoi(dig));
            }

            printResult(res, startEnd.first, startEnd.second);
        }else {
            cout << "Не удалось получить результат от сервера" << endl;
        }
    }

    close(client_fd);
    exit(0);
}



// Отправка графа на сервер по UDP
void sendGraphUDP(int fd, struct sockaddr_in& server_addr, socklen_t addr_len,
                  const vector<vector<int>>& graph, int start, int end) {
    // Отправляем размер графа
    auto size = graph.size();
    sendto(fd, &size, sizeof(size), 0,
           (struct sockaddr*)&server_addr, addr_len);

    // Отправляем списки смежности
    for (const auto& vertices : graph) {
        auto list_size = vertices.size();
        sendto(fd, &list_size, sizeof(list_size), 0,
               (struct sockaddr*)&server_addr, addr_len);
        sendto(fd, vertices.data(), list_size * sizeof(int), 0,
               (struct sockaddr*)&server_addr, addr_len);
    }

    // Отправляем начальную и конечную вершины
    sendto(fd, &start, sizeof(start), 0,
           (struct sockaddr*)&server_addr, addr_len);
    sendto(fd, &end, sizeof(end), 0,
           (struct sockaddr*)&server_addr, addr_len);
}