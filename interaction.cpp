#include <vector>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sstream>
#include <fstream>

#include "interaction.h"

#include "tcpClient.h"
#include "udpClient.h"

using namespace std;

// Функция ввода терминала
void scanfAll(char *buffer, const int &bufferLen, FILE *stream) {
    fgets(buffer, bufferLen, stream);
    buffer[strcspn(buffer, "\n")] = '\0';

    // ОЧИСТКА БУФЕРА если ввели больше символов
    if (strlen(buffer) == bufferLen - 1) {
        int ch;
        while ((ch = fgetc(stream)) != '\n' && ch != EOF);
    }
}

// Отключение клиента (TCP)
void exitTCP(int &fd, const char *buffer, const int &bufferLen) {
    if (strcmp(buffer, "exit") == 0) {
        send(fd, buffer, bufferLen, 0);
        printf("Клиент покинул сервер");
        close(fd);
        exit(0);
    }
}

// Отключение клиента (UDP)
void exitUDP(const char *buffer) {
    if (strcmp(buffer, "exit") == 0) {
        printf("Клиент покинул сервер");
        exit(0);
    }
}

// Запрос клиента описания графа
int inputType(char *buffer, const int &bufferLen, bool &flag) {
    int type = -1;
    while (type == -1) {
        cout << "Пожалуйста, выберите способ задания графа: 'input' или 'read' (из файла)?" << endl;
        cout << "(P.s. пишите слово в одинарных ковычках ''!)" << endl;
        cout << "Input << ";
        scanfAll(buffer, MAX_LEN, stdin);

        if (strcmp(buffer, "input") == 0) {type = 0;}
        else if (strcmp(buffer, "read") == 0) {type = 1;}
        else if (strcmp(buffer, "exit") == 0) {flag = false; return -1;}
    }
    return type;
}

// Вывод результата работы алгоритма клиенту
void printResult(pair<int, vector<int>> &result, int &start, int &end) {
    // Вывод расстояний
    int dist = result.first;
    vector<int> path = result.second;
    //auto n = dist.size();
    cout << "Результат длины пути (от " << start << " до " << end << "): ";
    // Точечная длина
    if (dist == -1) cout << "Вершины не соединены!\n";
    else cout << dist << '\n';
    /*
    // Перебор всех вариантов
    for (int i = 0; i < n; ++i) {
        if (dist[i] == -1) cout << i << ": INF\n";
        else cout << i << ": " << dist[i] << '\n';
    }
    */

    // Восстановление и вывод путей
    cout << "Результат пути (из " << start << " до " << end << "): ";
    if (dist == -1) cout << " путь между вершинами не существует";
    else {
        for (size_t j = 0; j < path.size(); ++j) {
            if (j) cout << "->";
            cout << path[j];
        }
        cout << '\n';
    }
    cout << endl;
    /*
    // Перебор всех вариантов
    for (int i = 0; i < n; ++i) {
        if (dist[i] == -1) {
            cout << i << ": no path\n";
            continue;
        }
        vector<int> path;
        for (int v = i; v != -1; v = parent[v]) path.push_back(v);
        reverse(path.begin(), path.end());
        cout << i << ": ";
        for (size_t j = 0; j < path.size(); ++j) {
            if (j) cout << "->";
            cout << path[j];
        }
        cout << '\n';
    }
    */
}

// Ручной ввод описания графа
void inputGraphTCP(int &fd, vector<vector<int>> &graph) {
    int n = -1;
    int vert;
    char buffer[4] = {0};
    while (true) {
        cout << "Введите число вершин: ";
        scanfAll(buffer, MAX_LEN, stdin);

        exitTCP(fd, buffer, MAX_LEN);

        if (isTwoDigit(buffer)) {n = atoi(buffer); break;}
    }
    graph.resize(n);
    for (int i = 0; i < n; i++) {
        while (true) {
            cout << "Как много вершин соединено с вершиной " << i << "? ";
            bzero(buffer, 4);
            scanfAll(buffer, MAX_LEN, stdin);

            exitTCP(fd, buffer, MAX_LEN);

            if (isTwoDigit(buffer)) {vert = atoi(buffer); break;}
        }
        for (int j = 0; j < vert; j++) {

            while (true) {
                cout << j + 1 << " соединена с " << i << ": ";
                bzero(buffer, 4);
                scanfAll(buffer, MAX_LEN, stdin);

                exitTCP(fd, buffer, MAX_LEN);

                if (isTwoDigit(buffer)) {graph[i].push_back(atoi(buffer)); break;}
            }
        }
    }
}


void inputGraphUDP(vector<vector<int> > &graph) {
    int n = -1;
    int vert;
    char buffer[4] = {0};
    while (true) {
        cout << "Введите число вершин: ";
        scanfAll(buffer, MAX_LEN, stdin);

        exitUDP(buffer);

        if (isTwoDigit(buffer)) {n = atoi(buffer); break;}
    }
    graph.resize(n);
    for (int i = 0; i < n; i++) {
        while (true) {
            cout << "Как много вершин соединено с вершиной " << i << "? ";
            bzero(buffer, 4);
            scanfAll(buffer, MAX_LEN, stdin);

            exitUDP(buffer);

            if (isTwoDigit(buffer)) {vert = atoi(buffer); break;}
        }
        for (int j = 0; j < vert; j++) {

            while (true) {
                cout << j + 1 << " соединена с " << i << ": ";
                bzero(buffer, 4);
                scanfAll(buffer, MAX_LEN, stdin);

                exitUDP(buffer);

                if (isTwoDigit(buffer)) {graph[i].push_back(atoi(buffer)); break;}
            }
        }
    }
}

// Чтение описания графа из файла (для TCP)
void readGraphTCP(int &fd, vector<vector<int>> &graph) {
    int l = 128;
    char filename[128] = {0};

    cout << "Введите название файла: ";
    scanfAll(filename, l, stdin);

    exitTCP(fd, filename, l);

    const char *ex = "exit";

    try {
        // Открываем файл для чтения
        ifstream file(filename);
        if (!file.is_open()) {
            throw runtime_error("Не удалось открыть файл: " + string(filename));
        }

        string line;

        // Читаем файл построчно
        while (getline(file, line)) {
            if (line.empty()) continue; // Пропускаем пустые строки

            vector<int> row;
            stringstream ss(line);
            string token;

            while (ss >> token) {
                // Проверка: все символы должны быть цифрами
                for (char c : token) {
                    if (!isdigit(c)) {
                        cout << "Неверный формат ввода графа" << endl;
                        exitTCP(fd, ex, 4);
                    }
                }
                row.push_back(std::stoi(token));
            }

            if (!row.empty()) {
                graph.push_back(row);
            }
        }
        file.close();
    }
    catch (const exception& e) {
        // Обработка ошибок
        printf("Ошибка при чтении графа: %s\n", e.what());
        exitTCP(fd, ex, 4);
    }
}

// Чтение описания графа из файла (для UDP)
void readGraphUDP(vector<vector<int>> &graph) {
    int l = 128;
    char filename[128] = {0};

    cout << "Введите название файла: ";
    scanfAll(filename, l, stdin);

    exitUDP(filename);

    const char *ex = "exit";

    try {
        // Открываем файл для чтения
        ifstream file(filename);
        if (!file.is_open()) {
            throw runtime_error("Не удалось открыть файл: " + string(filename));
        }

        string line;

        // Читаем файл построчно
        while (getline(file, line)) {
            if (line.empty()) continue; // Пропускаем пустые строки

            vector<int> row;
            stringstream ss(line);
            string token;

            while (ss >> token) {
                // Проверка: все символы должны быть цифрами
                for (char c : token) {
                    if (!isdigit(c)) {
                        cout << "Неверный формат ввода графа" << endl;
                        exitUDP(ex);
                    }
                }
                row.push_back(std::stoi(token));
            }

            if (!row.empty()) {
                graph.push_back(row);
            }
        }
        file.close();
    }

    catch (const exception& e) {
        // Обработка ошибок
        printf("Ошибка при чтении графа: %s\n", e.what());
        exitUDP(ex);

    }
}

// Ввод начальной и конечной вершины
pair<int, int> startEndNode(char *buffer) {
    int numberLen = 2   + 1;
    char start[numberLen];
    char end[numberLen];

    bzero(start, numberLen);
    bzero(end, numberLen);
    while (true) {
        printf("Введите начальную вершину (двухзначное число): ");
        scanfAll(start, numberLen, stdin);
        if (isTwoDigit(start)) break;
    }
    while (true) {
        printf("Введите конечную вершину (двухзначное число): ");
        scanfAll(end, numberLen, stdin);
        if (isTwoDigit(end)) break;
    }

    snprintf(buffer, sizeof(buffer), "%c%c/%c%c",
             start[0], start[1], end[0], end[1]);

    return make_pair(atoi(start), atoi(end));
}



// Проверка на однозначное-двузначное число (по ТЗ число <= 20, но реализовано на <100)
bool isTwoDigit(const char *buffer) {
    bool flag = true;
    if (!isdigit(buffer[1])) return isdigit(buffer[0]);
    for (int i = 0; i < 2; i++) {flag *= isdigit(buffer[i]);}
    return flag;
}
