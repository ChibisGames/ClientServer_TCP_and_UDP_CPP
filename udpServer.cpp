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

#include "wrappers.h"
#include "dijkstra.h"
#include "udpServer.h"


extern int MAX_LEN;

int UDP_PORT = 23101;
//#include <ctime>

using namespace std;

void startUdpServer() {
    // Открытие UDP сокета
    int udp_fd = Socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(UDP_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    constexpr int reuse = 1;
    if (setsockopt(udp_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        close(udp_fd);
        return;
    }

    Bind(udp_fd, (struct sockaddr *) &addr, sizeof addr);

    printf("UDP сервер начал работу на порте %d\n", UDP_PORT);

    char buffer[MAX_LEN];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    vector<vector<int>> graph(1);
    pair<long long, vector<int>> res;
    int start, end;

    while (true) {
        bzero(buffer, MAX_LEN);
        bzero(&client_addr, sizeof(client_addr));

        // Получение данных от клиента
        ssize_t recv_len = recvfrom(udp_fd, buffer, MAX_LEN, 0,
                                    (struct sockaddr*)&client_addr, &addr_len);

        if (recv_len <= 0) {
            continue;
        }

        printf("Получен запрос от клиента: %s:%d\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Обработка команды выхода
        if (strcmp(buffer, "exit") == 0) {
            printf("Клиент отключился\n");
            continue;
        }

        // Получение графа
        if (strcmp(buffer, "graph") == 0) {
            // Подтверждаем получение команды
            sendto(udp_fd, "OK", 2, 0,
                   (struct sockaddr*)&client_addr, addr_len);

            // Получаем размер графа
            size_t graph_size;
            recvfrom(udp_fd, &graph_size, sizeof(graph_size), 0,
                    (struct sockaddr*)&client_addr, &addr_len);

            graph.resize(graph_size);

            // Получаем списки смежности
            for (size_t i = 0; i < graph_size; ++i) {
                size_t list_size;
                recvfrom(udp_fd, &list_size, sizeof(list_size), 0,
                        (struct sockaddr*)&client_addr, &addr_len);

                graph[i].resize(list_size);
                recvfrom(udp_fd, graph[i].data(), list_size * sizeof(int), 0,
                        (struct sockaddr*)&client_addr, &addr_len);
            }

            // Получаем начальную и конечную вершины
            recvfrom(udp_fd, &start, sizeof(start), 0,
                    (struct sockaddr*)&client_addr, &addr_len);
            recvfrom(udp_fd, &end, sizeof(end), 0,
                    (struct sockaddr*)&client_addr, &addr_len);

            // Выполняем алгоритм Дейкстры
            res = dijkstra(graph, start, end);

            // Отправляем результат
            string result_str = to_string(res.first) + ",/";
            for (const auto& v : res.second) {
                result_str += to_string(v) + ",";
            }

            sendto(udp_fd, result_str.c_str(), result_str.length() + 1, 0,
                   (struct sockaddr*)&client_addr, addr_len);

            printf("Результат отправлен клиенту\n");
        }
    }

    close(udp_fd);
    printf("Отключение сервера\n");
    exit(0);
}