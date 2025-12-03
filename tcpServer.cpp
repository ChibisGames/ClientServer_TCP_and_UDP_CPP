#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <future>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <mutex>
#include <string>

#include "tcpServer.h"
#include "interaction.h"
#include "wrappers.h"
#include "dijkstra.h"

extern int MAX_LEN;

int TCP_PORT = 23101;
int MAX_LISTEN = 7;
//#include <ctime>

using namespace std;

mutex cout_mutex;

void startTcpServer() {
    // Открытие tcp сокета
    int tcp_fd = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(TCP_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    constexpr int reuse = 1;
    if (setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        close(tcp_fd);
        return;
    }

    Bind(tcp_fd, (struct sockaddr *) &addr, sizeof addr);
    Listen(tcp_fd, MAX_LISTEN);

    vector<thread> client_threads;

    {
        lock_guard<mutex> lock(cout_mutex);
        printf("TCP сервер начал работу на порте %d\n", TCP_PORT);
    }

    while (true) {
        struct sockaddr_in clientAddr = {0};
        socklen_t cl_size = sizeof(clientAddr);

        int clientfd = Accept(tcp_fd, (struct sockaddr *) &clientAddr, &cl_size);

        if (clientfd < 0) {
            continue;
        }

        // Создаем новый поток для обработки клиента
        client_threads.emplace_back([clientfd, clientAddr]() {
            handleTcpClient(clientfd, clientAddr);
        });
    }
    client_threads.clear();

    // Ждем завершения всех клиентских потоков
    for (auto& thread : client_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    close(tcp_fd);
    {
        lock_guard<mutex> lock(cout_mutex);
        printf("Отключение сервера\n");
    }
    exit(0);
}

void handleTcpClient(int clientfd, struct sockaddr_in clientAddr) {
    {
        lock_guard<mutex> lock(cout_mutex);
        printf("Соединение с клиентом по сокету: %d\n", clientfd);
    }

    //time_t rawtime;
    //time(&rawtime);

    char buffer[MAX_LEN];
    vector<vector<int>> graph(1);
    pair<long long, vector<int>> res;
    int start;
    int end;
    ssize_t brecv = 1;

    while (brecv) {
        bzero(buffer, MAX_LEN);

        brecv = Recv(clientfd, buffer, MAX_LEN, 0);

        if (brecv == 1) {
            graph = receiveGraphTCP(clientfd, graph, start, end);

            res = dijkstra(graph, start, end);

            sendResultTCP(clientfd, res);
        }
    }

    close(clientfd);
}

// Приём графа на сервере:
vector<vector<int>> &receiveGraphTCP(int &fd, vector<vector<int>> &graph, int &start, int &end) {
    size_t size;
    //time_t rawtime;
    //time(&rawtime);
    //cout << ctime(&rawtime) << endl;
    recv(fd, &size, sizeof(size), 0);

    graph.resize(size);
    for (size_t i = 0; i < size; ++i) {
        size_t list_size;
        recv(fd, &list_size, sizeof(list_size), 0);

        graph[i].resize(list_size);
        recv(fd, graph[i].data(), list_size * sizeof(int), 0);
    }
    recv(fd, &start, sizeof(start), 0);
    recv(fd, &end, sizeof(end), 0);
    return graph;
}

void sendResultTCP(const int &fd, const pair<int, vector<int>>& input) {
    string buffer;
    /*
    // Перебор всех длин от старта
    // const pair<vector<long long>, vector<int>>& input
    for (auto v : input.first) {
        buffer += to_string(v) + ",";
    }*/
    buffer += to_string(input.first);
    buffer += ",/";
    for (const auto v : input.second) {
        buffer+= to_string(v) + ",";
    }
    cout << buffer << endl;
    send(fd, buffer.c_str(), buffer.length() + 1, 0);
}

