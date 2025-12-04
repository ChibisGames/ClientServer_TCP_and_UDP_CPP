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

#include "tcpClient.h"
#include "validation.h"
#include "interaction.h"
#include "wrappers.h"

extern int MAX_LEN;

int TCP_PORT = 23101;
int MAX_LISTEN = 7;
//#include <ctime>

using namespace std;

mutex cout_mutex;

// Функция запуска клиента (TCP)
void startTcpClient(const char *ip, int port) {
    int clientfd = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    Connect(clientfd, (struct sockaddr*)&addr, sizeof addr);

    Inet_pton(AF_INET, ip, (void*)&addr.sin_addr);

    ssize_t brecv;
    char buffer[MAX_LEN];
    pair<int, vector<int>> res;
    pair<int, int> startEnd;

    bool activity_client = true;
    int inputTp;
    vector<vector<int>> graph;
    while(activity_client) {
        bzero(buffer, MAX_LEN);
        inputTp = inputType(buffer, MAX_LEN, activity_client);

        exitTCP(clientfd, buffer, MAX_LEN);
        graph.clear();
        if (!inputTp) {
            inputGraphTCP(clientfd, graph);
        }else {
            readGraphTCP(clientfd, graph);
        }

        bzero(buffer, MAX_LEN);
        startEnd = startEndNode(buffer); //Осторожно! Меняет buffer (хотя всё равно)!

        // Как буд-то невозможно, чтобы TCP что-то потерял
        // Проверяем на минимальное и максимальное кол-во вершин и рёбер
        if (!numberVertexes(graph, 1)) continue;
        // Проверяем на вхождение вершин в граф
        if (!isVertexesInBorder(startEnd.first, startEnd.second, graph, 1)) continue;
        // Проверка на связность графа (вершины должны быть соединены друг с другом, если одна соединена с другой)
        if (!connectivityVertexes(graph, 1)) continue;


        send(clientfd, buffer, MAX_LEN, 0);
        sendGraphTCP(clientfd, graph, startEnd.first, startEnd.second);
        bzero(buffer, MAX_LEN);
        receiveResultTCP(clientfd, buffer, MAX_LEN, res);
        printResult(res, startEnd.first, startEnd.second);
    }

    close(clientfd);
    exit(0);
}

// Отправка графа на сервер
void sendGraphTCP(const int &fd, const vector<vector<int>>& graph, const int &start, const int &end) {
    // Сначала отправляем размер графа (количество вершин)
    auto size = graph.size();
    //time_t rawtime;
    //time(&rawtime);
    //cout << ctime(&rawtime) << endl;
    send(fd, &size, sizeof(size), 0);

    // Отправляем размеры списков смежности и сами списки
    for (const auto& vertices : graph) {
        auto list_size = vertices.size();
        send(fd, &list_size, sizeof(list_size), 0);
        send(fd, vertices.data(), list_size * sizeof(int), 0);
    }
    send(fd, &start, sizeof(start), 0);
    send(fd, &end, sizeof(end), 0);
}

pair<int, vector<int>> receiveResultTCP(int &fd, char *buffer, const int &bufferLen,
                                                      pair<int, vector<int>> &input) {
    ssize_t bresv;
    bresv = recv(fd, buffer, bufferLen, 0);
    if (bresv <= 0) {
        perror("Потеряна связь с сервером");
        exit(EXIT_FAILURE);
    }
    exitTCP(fd, buffer, bufferLen);

    size_t i = 0;
    string dig;
    input.second.clear();
    for (i; i < bufferLen; ++i) {
        if (const char ch = buffer[i]; ch == '/') {
            break;
        }else {
            if (ch == ',') {input.first = stoll(dig); dig = ""; continue;}
            dig += ch;
        }
    }
    i++;
    for (i; i < bufferLen; ++i) {
        if (const char ch = buffer[i];ch == ',') {input.second.push_back(stoi(dig)); dig = ""; continue;}
        else dig += ch;
    }
    return input;
}