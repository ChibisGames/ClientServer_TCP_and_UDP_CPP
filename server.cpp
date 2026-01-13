#include "tcpServer.h"
#include "udpServer.h"
#include "validation.h"


int main(int argc, char *argv[]) {
    // в argv[0] храниться абсолютный путь?
    // в argv[1] храниться протокол (tcp/udp)
    // в argv[2] хранится номер порта

    //startUdpServer();
    //startTcpServer();
    int port;
    if (isPort(argv[2])) {
        port = atoi(argv[2]);
        printf("Запуск сервера на порту по умолчанию");
    }
    else port = 23101;

    if (strcmp(argv[1], "tcp") == 0) {
        startTcpServer(port);
    } else if (strcmp(argv[1], "udp") == 0) {
        startUdpServer(port);
    }
    return 0;
}