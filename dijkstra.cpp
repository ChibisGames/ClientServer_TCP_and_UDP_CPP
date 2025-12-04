#include <vector>
#include <iostream>
#include <algorithm>

#include "dijkstra.h"
using namespace std;
// граф: 5 вершин (0..4)
// ребра: 0-1, 0-2, 1-2, 1-3, 3-4
/*
vector<vector<int>> adj = {
    {1, 2},    // смежные вершины для 0
    {0, 2, 3}, // для 1
    {0, 1},    // для 2
    {1, 4},    // для 3
    {3}        // для 4
};
*/

// Алгоритм Дейкстры
pair<int, vector<int>> dijkstra(vector<vector<int>>& graph, int &start, int &end) {
    auto n = graph.size();

    const long long INF = (1LL<<60);
    vector<long long> dist(n, INF);
    vector<int> parent(n, -1);
    dist[start] = 0;

    // Используем priority_queue по парам (distance, vertex)
    priority_queue<pair<long long,int>, vector<pair<long long,int>>, greater<>> pq;
    pq.emplace(0, start);

    while (!pq.empty()) {
        auto p = pq.top(); pq.pop();
        long long d = p.first;
        int v = p.second;
        if (d != dist[v]) continue;
        for (int to : graph[v]) {
            int w = 1; // вес ребра
            if (dist[to] > dist[v] + w) {
                dist[to] = dist[v] + w;
                parent[to] = v;
                pq.emplace(dist[to], to);
            }
        }
    }
    for (auto &v : dist) {
        if (v == INF) {v = -1;}
    }
    // Чтобы оптимизировать сообщение, вместо массива, по которому можно восстановить
    // путь, сразу вычисляем путь (сам путь <= массиву)
    vector<int> path;
    for (int v = end; v != -1; v = parent[v]) path.push_back(v);
    ranges::reverse(path);

    pair<int, vector<int>> out;
    // Так же для оптимизации возвращаем только длину пути (+ экономия на месте,
    // так как вместо vector<long long> мы хранм просто int)
    out.first = static_cast<int>(dist[end]); // длина пути
    out.second = path;
    return out;
}