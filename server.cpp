#include "tcpServer.h"
#include "udpServer.h"


int main(int argc, char *argv[]) {
    // в argv[0] храниться абсолютный путь?
    // в argv[1] храниться протокол (tcp/udp)

    //startUdpServer();
    //startTcpServer();

    if (strcmp(argv[1], "tcp") == 0) {
            startTcpServer();
        } else if (strcmp(argv[1], "udp") == 0) {
            startUdpServer();
        }
    return 0;
    }
