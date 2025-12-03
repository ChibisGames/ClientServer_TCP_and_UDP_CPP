#ifndef CLIENTSERVER_TCPSERVER_H
#define CLIENTSERVER_TCPSERVER_H

#include <vector>



using namespace std;


void startTcpServer();

void handleTcpClient(int clientfd, struct sockaddr_in clientAddr);

vector<vector<int>> &receiveGraphTCP(int &fd, vector<vector<int>> &graph, int &start, int &end);

void sendResultTCP(const int &fd, const pair<int, vector<int>>& input);


#endif //CLIENTSERVER_TCPSERVER_H