#ifndef CLIENTSERVER_WRAPPERS_H
#define CLIENTSERVER_WRAPPERS_H

#include <sys/types.h>
#include <sys/socket.h>

int Socket(int domain,int type, int protocol);

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

void Listen(int port, int backlog);

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

void Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

void Inet_pton(int af, const char *src, void *dst);

void Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout);

int Recv(int &fd, char *buffer, size_t bufferLen, int flag);

int Recvfrom(int &fd, char *buffer, size_t bufferLen, int flag,
    struct sockaddr_in &serverAddr, socklen_t &serverAddrLen);


#endif //CLIENTSERVER_WRAPPERS_H