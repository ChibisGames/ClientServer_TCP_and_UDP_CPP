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
#include <sys/time.h>

#include "interaction.h"
#include "wrappers.h"
#include "validation.h"
#include "udpClient.h"

extern int MAX_LEN;

using namespace std;

// Функция отправки графа на сервер по UDP с подтверждениями
bool sendGraphUDPWithAck(int fd, struct sockaddr_in& server_addr, socklen_t addr_len,
                        const vector<vector<int>>& graph, int start, int end) {
    char buffer[MAX_LEN];

    // Три попытки отправки команды "graph" с ожиданием подтверждения
    bool received_ok = false;
    int attempts = 0;
    const int max_attempts = 3;
    const int timeout_seconds = 3;

    while (attempts < max_attempts && !received_ok) {
        attempts++;

        // Отправляем команду "graph"
        strcpy(buffer, "graph");
        ssize_t sent_bytes = sendto(fd, buffer, strlen(buffer), 0,
                                    (struct sockaddr*)&server_addr, addr_len);

        if (sent_bytes <= 0) {
            cout << "Попытка " << attempts << ": Ошибка отправки команды" << endl;
            continue;
        }

        // Устанавливаем таймаут 3 секунды для получения подтверждения
        struct timeval tv;
        tv.tv_sec = timeout_seconds;
        tv.tv_usec = 0;
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        bzero(buffer, MAX_LEN);
        socklen_t temp_addr_len = addr_len;
        ssize_t recv_len = recvfrom(fd, buffer, MAX_LEN, 0,
                                    (struct sockaddr*)&server_addr, &temp_addr_len);

        // Сбрасываем таймаут после попытки
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        if (recv_len > 0) {
            if (strcmp(buffer, "OK") == 0) {
                received_ok = true;
                cout << "Получено подтверждение от сервера" << endl;
            } else {
                cout << "Попытка " << attempts << ": Получен неожиданный ответ: " << buffer << endl;
            }
        } else if (recv_len == 0) {
            cout << "Попытка " << attempts << ": Соединение закрыто сервером" << endl;
        } else {
            if (attempts < max_attempts) {
                cout << "Попытка " << attempts << ": Таймаут ожидания подтверждения ("
                     << timeout_seconds << " сек). Повторная попытка..." << endl;
            } else {
                cout << "Попытка " << attempts << ": Таймаут ожидания подтверждения ("
                     << timeout_seconds << " сек)." << endl;
            }
        }

        // Небольшая пауза между попытками (кроме последней)
        if (attempts < max_attempts && !received_ok) {
            sleep(1);
        }
    }

    if (!received_ok) {
        cout << "Не удалось получить подтверждение от сервера" << endl;
        return false;
    }

    // Отправляем размер графа
    size_t graph_size = graph.size();
    sendto(fd, &graph_size, sizeof(graph_size), 0,
           (struct sockaddr*)&server_addr, addr_len);

    // Отправляем списки смежности
    for (const auto& vertices : graph) {
        size_t list_size = vertices.size();
        sendto(fd, &list_size, sizeof(list_size), 0,
               (struct sockaddr*)&server_addr, addr_len);

        // Проверяем, что список не пустой перед отправкой данных
        if (list_size > 0) {
            sendto(fd, vertices.data(), list_size * sizeof(int), 0,
                   (struct sockaddr*)&server_addr, addr_len);
        } else {
            // Отправляем пустой список
            sendto(fd, nullptr, 0, 0,
                   (struct sockaddr*)&server_addr, addr_len);
        }
    }

    // Отправляем начальную и конечную вершины
    sendto(fd, &start, sizeof(start), 0,
           (struct sockaddr*)&server_addr, addr_len);
    sendto(fd, &end, sizeof(end), 0,
           (struct sockaddr*)&server_addr, addr_len);

    return true;
}

// Функция получения результата от сервера
bool receiveResultUDP(int fd, struct sockaddr_in& server_addr, socklen_t addr_len,
                     pair<int, vector<int>>& result) {
    char buffer[MAX_LEN];

    // Устанавливаем таймаут для получения результата
    struct timeval tv;
    tv.tv_sec = 10; // Увеличиваем таймаут для получения результата
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    bzero(buffer, MAX_LEN);
    ssize_t recv_len = recvfrom(fd, buffer, MAX_LEN, 0,
                                (struct sockaddr*)&server_addr, &addr_len);

    // Сбрасываем таймаут
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    if (recv_len <= 0) {
        cout << "Не удалось получить результат от сервера (таймаут)" << endl;
        return false;
    }

    // Добавляем нуль-терминатор
    if (recv_len < MAX_LEN) {
        buffer[recv_len] = '\0';
    } else {
        buffer[MAX_LEN - 1] = '\0';
    }

    //cout << "DEBUG: Получен результат: " << buffer << endl; // Отладочная печать

    // Проверяем на ошибку
    if (strncmp(buffer, "ERROR", 5) == 0) {
        cout << "Сервер вернул ошибку: некорректные данные графа" << endl;
        return false;
    }

    // Парсим результат
    string received_str(buffer);
    result.second.clear();

    // Ищем разделитель между длиной и путем
    size_t slash_pos = received_str.find("/");
    if (slash_pos == string::npos) {
        cout << "Некорректный формат ответа от сервера" << endl;
        return false;
    }

    // Извлекаем длину пути (часть до /)
    string dist_str = received_str.substr(0, slash_pos);
    // Убираем возможные запятые
    size_t comma_pos = dist_str.find(",");
    if (comma_pos != string::npos) {
        dist_str = dist_str.substr(0, comma_pos);
    }

    try {
        if (dist_str.empty()) {
            result.first = -1;
        } else {
            result.first = stoi(dist_str);
        }
    } catch (const std::exception& e) {
        cout << "Ошибка парсинга длины пути: " << e.what() << endl;
        result.first = -1;
    }

    // Извлекаем путь (часть после /)
    string path_str = received_str.substr(slash_pos + 1);

    // Парсим путь, разделенный запятыми
    if (!path_str.empty()) {
        size_t start = 0;
        size_t end = path_str.find(",");

        while (end != string::npos) {
            string num_str = path_str.substr(start, end - start);
            if (!num_str.empty()) {
                try {
                    result.second.push_back(stoi(num_str));
                } catch (const std::exception& e) {
                    cout << "Ошибка парсинга вершины пути: " << num_str << endl;
                }
            }
            start = end + 1;
            end = path_str.find(",", start);
        }

        // Последний элемент
        string last_num_str = path_str.substr(start);
        if (!last_num_str.empty()) {
            try {
                result.second.push_back(stoi(last_num_str));
            } catch (const std::exception& e) {
                cout << "Ошибка парсинга последней вершины: " << last_num_str << endl;
            }
        }
    }

    return true;
}

// Функция запуска клиента (UDP) - обновленная версия
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

    cout << "UDP клиент запущен. Подключение к " << ip << ":" << port << endl;

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
        if (inputTp == 0) {
            cout << "Выбран ручной ввод графа" << endl;
            inputGraphUDP(graph);
        } else if (inputTp == 1) {
            cout << "Выбран ввод графа из файла" << endl;
            readGraphUDP(graph);
        } else {
            continue;
        }

        if (graph.empty()) {
            cout << "Граф пуст. Попробуйте снова." << endl;
            continue;
        }

        cout << "Граф успешно загружен. Вершин: " << graph.size() << endl;

        bzero(buffer, MAX_LEN);
        startEnd = startEndNode(buffer);

        cout << "Начальная вершина: " << startEnd.first << ", конечная: " << startEnd.second << endl;

        // Проверяем на минимальное и максимальное кол-во вершин и рёбер
        if (!numberVertexes(graph, 1)) {
            cout << "Попробуйте снова с корректными данными графа" << endl;
            continue;
        }

        // Проверяем на вхождение вершин в граф
        if (!isVertexesInBorder(startEnd.first, startEnd.second, graph, 1)) {
            cout << "Попробуйте снова с корректными вершинами" << endl;
            continue;
        }

        // Проверка на связность графа
        if (!connectivityVertexes(graph, 1)) {
            cout << "Попробуйте снова с корректными связями вершин" << endl;
            continue;
        }

        cout << "Отправка данных на сервер..." << endl;

        // Отправляем граф и получаем подтверждение
        if (!sendGraphUDPWithAck(client_fd, server_addr, addr_len, graph,
                                startEnd.first, startEnd.second)) {
            cout << "Ошибка отправки данных. Попробуйте снова." << endl;
            continue;
        }

        cout << "Данные отправлены успешно. Ожидание результата от сервера..." << endl;

        // Получаем результат
        if (receiveResultUDP(client_fd, server_addr, addr_len, res)) {
            printResult(res, startEnd.first, startEnd.second);
        } else {
            cout << "Не удалось получить результат. Сервер может быть перегружен." << endl;
        }

        // Небольшая пауза между запросами
        sleep(1);
    }

    close(client_fd);
    exit(0);
}