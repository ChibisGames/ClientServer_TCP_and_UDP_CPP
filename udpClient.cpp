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
#include "dijkstra.h"
#include "udpClient.h"

extern int MAX_LEN;

int UDP_PORT = 23101;
//#include <ctime>

using namespace std;

struct UDPPacket {
    vector<char> buffer= vector<char>(MAX_LEN);
    struct sockaddr_in client_addr;
    socklen_t addr_len;
};

void startUdpClient() {
    int client_fd = Socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_PORT);
    Inet_pton(AF_INET, "127.0.0.1", (void*)&server_addr.sin_addr);

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
            //inputGraphUDP(graph);
        } else {
            readGraphUDP(graph);
        }

        bzero(buffer, MAX_LEN);
        startEnd = startEndNode(buffer);

        // Отправляем команду "graph" серверу
        strcpy(buffer, "graph");
        sendto(client_fd, buffer, strlen(buffer), 0,
               (struct sockaddr*)&server_addr, addr_len);

        // Ждем подтверждения
        bzero(buffer, MAX_LEN);
        recvfrom(client_fd, buffer, MAX_LEN, 0,
                (struct sockaddr*)&server_addr, &addr_len);

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
            // Парсим результат
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