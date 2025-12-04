#include <iostream>

#include "validation.h"
#include "interaction.h"


bool isIpv4(const char *ip) {
    int dotCounter = 0;
    string limit;
    for (int i = 1; i < strlen(ip); i++) {
        if (ip[i] == '.') {
            dotCounter++;
            if (atoi(limit.c_str()) > 255) return false;
            limit.clear();
        }
        else {
            if (!isdigit(ip[i])) return false;
            limit += ip[i];
        }
    }
    if (dotCounter != 3) return false;
    return true;
}

bool isPort(const char *word) {
    size_t size = strlen(word);
    if (word == nullptr || size == 0) {
        cout << "Пустая строка" << endl;
        return false;
    }

    for (int i = 0; i < size; i++) {
        if (!isdigit(word[i])) {
            cout << "Неверный" << endl;
            return 0;
        }
    }
    int port = atoi(word);
    if (port < 1024 || port > 65535) {
        cout << "Порт " << port << " вне диапазона 1024-65535" << endl;
        return false;
    }
    return true;
}


// programme: 0-server, 1-client!!!

// Проверяем на вхождение вершин в граф
bool isVertexesInBorder(int &start, int &end, vector<vector<int>> &graph, int programme) {
    size_t size = graph.size();
    // Хотя программа гарантирует >= 0 для вершин, но на всякий случай
    if (0 > start || start >= size) {
        if (programme == 1) cout << "Начальной вершины нет в графе!" << endl;
        return false;
    }
    if (0 > end || end >= size) {
        if (programme == 1) cout << "Конечной вершины нет в графе!" << endl;
        return false;
    }
    for (size_t i = 0; i < size; i++) {
        for (int j : graph[i]) {
            if (0 > j || j >= size) {
                if (programme == 1) cout << "Вершины не найдены в графе!" << endl;
                return false;
            }
        }
    }
    return true;
}

// Проверка на минимальное и максимальное кол-во вершин и рёбер
bool numberVertexes(vector<vector<int>> &graph, int programme) {
    if (graph.size() > 20) {
        if (programme == 1) cout << "Граф должен содержать не более 20 вершин" << endl;
        return false;
    }
    int edges = 0;
    for (int i = 0; i < graph.size(); i++) {
        for (const auto j : graph[i]) {
            if (j == i) edges++;
            else edges += 2; // считаем путь в 2-ух направлениях
        }
    }
    // Меньше 6 вершин - это понятно, а вот с рёбрами немного сложнее: так как граф неориентированный,
    // то вектор смежности учитывает 2 прохода между вершинами (петля случай отдельный) - значит поднимем лимит до 12
    if (graph.size() < 6 || edges < 12) {
        if (programme == 1) cout << "Граф должен содержать не менее 6 вершин и 6 рёбер" << endl;
        return false;
    }
    return true;
}

// Проверка на связность графа (вершины должны быть соединены друг с другом, если одна соединена с другой) и
// проверка на вершины-цифры
bool connectivityVertexes(vector<vector<int>> &graph, int programme) {
    for (int i = 0; i < graph.size(); i++) {
        for (const int j : graph[i]) {
            auto it = ranges::find(graph[j], i);
            if (it == graph[j].end()) {
                if (programme == 1) cout << "Неверный формат ввода графа (ошибка соединения вершин)" << endl;
                return false;
            }
        }
    }
    return true;
}