#include "tcpClient.h"
#include "udpClient.h"

int main(int argc, char *argv[]) {
    // в argv[0] храниться абсолютный путь?
    /*
    if (strcmp(argv[1], "tcp") == 0) {
        startTcpServer();
    } else if (strcmp(argv[1], "udp") == 0) {
        exit(0);
    }*/
    //startTcpClient();
    startUdpClient();
    return 0;
}