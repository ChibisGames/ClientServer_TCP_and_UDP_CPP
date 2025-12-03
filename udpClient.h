#ifndef CLIENTSERVER_UDPCLIENT_H
#define CLIENTSERVER_UDPCLIENT_H

#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;


void startUdpClient();

void sendGraphUDP(int fd, struct sockaddr_in& server_addr, socklen_t addr_len,
                  const vector<vector<int>>& graph, int start, int end);

#endif //CLIENTSERVER_UDPCLIENT_H