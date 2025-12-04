#ifndef CLIENTSERVER_VALIDATION_H
#define CLIENTSERVER_VALIDATION_H

#include <vector>

using namespace std;


bool isIpv4(const char *ip);

bool isPort(const char *word);

bool isVertexesInBorder(int &start, int &end, vector<vector<int>> &graph, int programme);

bool numberVertexes(vector<vector<int>> &graph, int programme);

bool connectivityVertexes(vector<vector<int>> &graph, int programme);
#endif //CLIENTSERVER_VALIDATION_H