#include <iostream>
#include "tcpClient.h"
#include "udpClient.h"
#include "validation.h"


int main(int argc, char *argv[]) {
    // в argv[0] храниться абсолютный путь?
    // в argv[1] храниться IP-адрес сервера
    // в argv[2] храниться протокол (tcp/udp)
    // в argv[3] храниться номер порта

    const char *ip = argv[1];
    if (!isIpv4(ip)) {cout << "Неверный формат IPv4" << endl; return 0;}
    int port = 0;
    if (isPort(argv[3])) port = atoi(argv[3]);

    if (strcmp(argv[2], "tcp") == 0) {
        startTcpClient(ip, port);
    } else if (strcmp(argv[2], "udp") == 0) {
        startUdpClient(ip, port);
    }
    //startTcpClient(ip, port);
    //startUdpClient(ip, port);
    return 0;
}