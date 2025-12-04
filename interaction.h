#ifndef CLIENTSERVER_INTERACTION_H
#define CLIENTSERVER_INTERACTION_H

#include <vector>

using namespace std;

void scanfAll(char *buffer, const int &bufferLen, FILE *stream);

void exitTCP(int &fd, const char *buffer, const int &bufferLen);

void exitUDP(const char *buffer);

int inputType(char *buffer, const int &bufferLen, bool &flag);

void printResult(pair<int, vector<int>> &result, int &start, int &end);

void inputGraphTCP(int &fd, vector<vector<int>> &graph);
void inputGraphUDP(vector<vector<int>> &graph);

void readGraphTCP(int &fd, vector<vector<int>> &graph);
void readGraphUDP(vector<vector<int>> &graph);

pair<int, int> startEndNode(char *buffer);


bool isTwoDigit(const char *buffer);
#endif //CLIENTSERVER_INTERACTION_H