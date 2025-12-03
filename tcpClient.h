#ifndef CLIENTSERVER_TCPCLIENT_H
#define CLIENTSERVER_TCPCLIENT_H

#include <vector>

extern int MAX_LEN;

using namespace std;


void startTcpClient();

void sendGraphTCP(const int &fd, const vector<vector<int>>& graph, const int &start, const int &end);

pair<int, vector<int>> receiveResultTCP(int &fd, char *buffer, const int &bufferLen,
                                                      pair<int, vector<int>> &input);

#endif //CLIENTSERVER_TCPCLIENT_H