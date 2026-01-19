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
#include <unordered_map>
#include <functional>
#include <fcntl.h>
#include <sys/time.h>
#include <ctime>

#include "wrappers.h"
#include "dijkstra.h"
#include "udpServer.h"
#include "validation.h"

extern int MAX_LEN;

using namespace std;

// Структура для хранения состояния клиента
struct ClientState {
    sockaddr_in addr;
    socklen_t addr_len;
    vector<vector<int>> graph;
    int start;
    int end;
    int stage; // 0 - ожидание, 1 - получение размера графа, 2 - получение списков смежности, 3 - получение вершин
    size_t current_vertex;
    size_t graph_size;
    size_t current_list_size;
    time_t last_activity;

    ClientState() : stage(0), current_vertex(0), graph_size(0),
                   current_list_size(0), last_activity(time(nullptr)) {}

    void reset() {
        graph.clear();
        stage = 0;
        current_vertex = 0;
        graph_size = 0;
        current_list_size = 0;
        last_activity = time(nullptr);
    }
};

// Хэш-функция для sockaddr_in
struct sockaddr_in_hash {
    size_t operator()(const sockaddr_in& addr) const {
        return hash<string>()(string(reinterpret_cast<const char*>(&addr), sizeof(addr)));
    }
};

// Функция сравнения sockaddr_in
struct sockaddr_in_equal {
    bool operator()(const sockaddr_in& a, const sockaddr_in& b) const {
        return a.sin_port == b.sin_port &&
               a.sin_addr.s_addr == b.sin_addr.s_addr;
    }
};

// Глобальные переменные для управления клиентами
unordered_map<sockaddr_in, ClientState, sockaddr_in_hash, sockaddr_in_equal> clients;
const int CLIENT_TIMEOUT = 30; // таймаут клиента в секундах

// Функция для удаления неактивных клиентов
void cleanup_inactive_clients() {
    time_t now = time(nullptr);
    vector<sockaddr_in> to_remove;
    // Последняя активность большие 30 секунд
    for (auto& [addr, state] : clients) {
        if (difftime(now, state.last_activity) > CLIENT_TIMEOUT) {
            to_remove.push_back(addr);
        }
    }
    // Лог в сервер
    for (const auto& addr : to_remove) {
        clients.erase(addr);
        printf("Клиент %s:%d удалён по таймауту\n",
               inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    }
}

// Обработка нового клиента или существующего
void handle_client_request(int udp_fd, char* buffer, ssize_t recv_len,
                          sockaddr_in& client_addr, socklen_t addr_len) {

    // Обновляем время активности
    if (clients.find(client_addr) != clients.end()) {
        clients[client_addr].last_activity = time(nullptr);
    }

    // Обработка команды выхода
    if (strcmp(buffer, "exit") == 0) {
        printf("Клиент %s:%d отключился\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        clients.erase(client_addr);
        return;
    }

    // Если это новый клиент или сброс состояния
    if (clients.find(client_addr) == clients.end() ||
        clients[client_addr].stage == 0) {

        if (strcmp(buffer, "graph") == 0) {
            ClientState new_state;
            new_state.addr = client_addr;
            new_state.addr_len = addr_len;
            new_state.stage = 1;
            new_state.last_activity = time(nullptr);
            clients[client_addr] = new_state;

            // Подтверждаем получение команды
            sendto(udp_fd, "OK", 2, 0,
                   (struct sockaddr*)&client_addr, addr_len);
            printf("Начата обработка графа для клиента %s:%d\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        }
        return;
    }

    // Обработка данных от существующего клиента
    ClientState& state = clients[client_addr];

    switch (state.stage) {
        case 1: // Получение размера графа
            if (recv_len == sizeof(size_t)) {
                memcpy(&state.graph_size, buffer, sizeof(size_t));
                state.graph.resize(state.graph_size);
                state.current_vertex = 0;
                state.stage = 2;
                printf("Клиент %s:%d: получен размер графа: %zu\n",
                       inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
                       state.graph_size);
            }
            break;

        case 2: // Получение размера списка смежности
            if (recv_len == sizeof(size_t)) {
                memcpy(&state.current_list_size, buffer, sizeof(size_t));
                state.graph[state.current_vertex].resize(state.current_list_size);
                state.stage = 3;
            }
            break;

        case 3: // Получение самого списка смежности
            if (recv_len == state.current_list_size * sizeof(int)) {
                memcpy(state.graph[state.current_vertex].data(), buffer, recv_len);
                state.current_vertex++;

                if (state.current_vertex < state.graph_size) {
                    state.stage = 2; // ждем следующий список
                } else {
                    state.stage = 4; // переходим к получению вершин
                }
            }
            break;

        case 4: // Получение начальной вершины
            if (recv_len == sizeof(int)) {
                memcpy(&state.start, buffer, sizeof(int));
                state.stage = 5;
            }
            break;

        case 5: // Получение конечной вершины
            if (recv_len == sizeof(int)) {
                memcpy(&state.end, buffer, sizeof(int));

                // Все данные получены, выполняем алгоритм
                printf("Клиент %s:%d: все данные получены, выполнение алгоритма...\n",
                       inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                // Проверки
                if (!numberVertexes(state.graph, 0) ||
                    !isVertexesInBorder(state.start, state.end, state.graph, 0) ||
                    !connectivityVertexes(state.graph, 0)) {

                    sendto(udp_fd, "ERROR", 5, 0,
                           (struct sockaddr*)&client_addr, addr_len);
                    printf("Клиент %s:%d: ошибка в данных графа\n",
                           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                } else {
                    // Выполняем алгоритм Дейкстры
                    auto res = dijkstra(state.graph, state.start, state.end);

                    // Отправляем результат
                    string result_str = to_string(res.first) + ",/";
                    for (const auto& v : res.second) {
                        result_str += to_string(v) + ",";
                    }

                    sendto(udp_fd, result_str.c_str(), result_str.length() + 1, 0,
                           (struct sockaddr*)&client_addr, addr_len);

                    printf("Результат отправлен клиенту %s:%d\n",
                           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                }

                // Сбрасываем состояние клиента
                state.reset();
            }
            break;
    }
}

// Функция запуска сервера (UDP) с поддержкой множества клиентов
void startUdpServer(int port) {
    // Открытие UDP сокета
    int udp_fd = Socket(AF_INET, SOCK_DGRAM, 0);

    // Установка неблокирующего режима
    int flags = fcntl(udp_fd, F_GETFL, 0);
    fcntl(udp_fd, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    constexpr int reuse = 1;
    if (setsockopt(udp_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        close(udp_fd);
        return;
    }

    Bind(udp_fd, (struct sockaddr *) &addr, sizeof addr);

    printf("UDP сервер начал работу на порте %d\n", port);

    fd_set read_fds;
    char buffer[MAX_LEN];

    while (true) {
        // Очистка неактивных клиентов
        cleanup_inactive_clients();

        // Настройка select
        FD_ZERO(&read_fds);
        FD_SET(udp_fd, &read_fds);

        struct timeval tv;
        tv.tv_sec = 1;  // Проверка каждую секунду для таймаута клиентов
        tv.tv_usec = 0;

        int activity = select(udp_fd + 1, &read_fds, NULL, NULL, &tv);

        if (activity < 0) {
            perror("select error");
            continue;
        }

        if (activity == 0) {
            // Таймаут, продолжаем цикл
            continue;
        }

        // Если есть данные для чтения
        if (FD_ISSET(udp_fd, &read_fds)) {
            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);

            bzero(buffer, MAX_LEN);
            bzero(&client_addr, sizeof(client_addr));

            ssize_t recv_len = recvfrom(udp_fd, buffer, MAX_LEN, 0,
                                        (struct sockaddr*)&client_addr, &addr_len);

            if (recv_len <= 0) {
                continue;
            }

            // Добавляем завершающий нуль
            if (recv_len < MAX_LEN) {
                buffer[recv_len] = '\0';
            } else {
                buffer[MAX_LEN - 1] = '\0';
            }

            printf("Получено %zd байт от %s:%d\n",
                   recv_len,
                   inet_ntoa(client_addr.sin_addr),
                   ntohs(client_addr.sin_port));

            // Обрабатываем запрос
            handle_client_request(udp_fd, buffer, recv_len, client_addr, addr_len);
        }
    }

    close(udp_fd);
    printf("Отключение сервера\n");
}