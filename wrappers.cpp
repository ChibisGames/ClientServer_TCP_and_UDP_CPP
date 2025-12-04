#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/_select.h>
#include <mutex>
#include <unistd.h>
#include <string>

using namespace std;


extern mutex cout_mutex;
int MAX_LEN = 1024;


//  Обёртка для создания сокета
int Socket(int domain,int type, int protocol) {
    int res = socket(domain, type, protocol);
    if (res == -1) {
        perror("socket is failed");
        exit(EXIT_FAILURE);
    }
    return res;
}

//  Обёртка для закрепления сокета
void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int res = bind(sockfd, addr, addrlen);
    if (res == -1) {
        perror("bind is failed");
        exit(EXIT_FAILURE);
    }
}

//  Обёртка для создания очереди сокета (для TCP)
void Listen(int port, int backlog) {
    int res = listen(port, backlog);
    if (res == -1) {
        perror("listen is failed");
        exit(EXIT_FAILURE);
    }
}

//  Обёртка для принятия сокетом сервера клиента (для TCP)
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int res = accept(sockfd, (struct sockaddr*)addr, addrlen);
    if (res == -1) {
        perror("accept is failed");
        exit(EXIT_FAILURE);
    }
    return res;
}

//  Обёртка для запроса клиента на подключения к сокету сервера (для TCP)
void Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int res = connect(sockfd, addr, addrlen);
    if (res == -1) {
        perror("connect is failed");
        exit(EXIT_FAILURE);
    }
}

// Обёртка для преобразования текстового представления IP-адреса
void Inet_pton(int af, const char *src, void *dst) {
    int res = inet_pton(af, src, dst);
    if (res == 0) {
        printf("net_pton failed: <== 0");
        exit(EXIT_FAILURE);
    }
    if (res == -1) {
        perror("inet_pton is failed");
        exit(EXIT_FAILURE);
    }
}

// Обёртка для системный вызов ввода/вывода, который позволяет мониторить несколько сокетов одновременно
void Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout) {
    if (select(nfds, readfds, writefds, errorfds, timeout) < 0) {
        perror("select failed");
        exit(EXIT_FAILURE);
    }
}

// Обёртка для принятия сообщения TCP
int Recv(int &fd, char *buffer, size_t bufferLen, int flag) {
    ssize_t brecv = recv(fd, buffer, bufferLen, flag);
    if (brecv <= 0) {
        // Клиент отключился
        {
            lock_guard<mutex> lock(cout_mutex);
            printf("Потеряна связь с клиентом %d\n", fd);
        }
        close(fd);
        return 0;
    }

    if (strcmp(buffer, "exit") == 0) {
        {
            lock_guard<mutex> lock(cout_mutex);
            printf("Клиент %d покинул сервер\n", fd);
        }
        close(fd);
        return 0;
    }

    /*{
        lock_guard<mutex> lock(cout_mutex);
        printf("Received message from client %d: %s\n", fd, buffer);
    }*/
    return 1;
}

// Обёртка для принятия сообщения UDp
int Recvfrom(int &fd, char *buffer, size_t bufferLen, int flag,
    struct sockaddr_in &serverAddr, socklen_t &serverAddrLen) {
    ssize_t brecv = recvfrom(fd, buffer, bufferLen, flag, (struct sockaddr *)&serverAddr, &serverAddrLen);
    if (brecv <= 0) {
        // Клиент отключился
        {
            lock_guard<mutex> lock(cout_mutex);
            printf("Потеряна связь с клиентом %d\n", fd);
        }
        close(fd);
        return 0;
    }

    if (strcmp(buffer, "exit") == 0) {
        {
            lock_guard<mutex> lock(cout_mutex);
            printf("Client %d exit server\n", fd);
        }
        close(fd);
        return 0;
    }

    {
        lock_guard<mutex> lock(cout_mutex);
        printf("Received message from client %d: %s\n", fd, buffer);
    }
    return 1;
}