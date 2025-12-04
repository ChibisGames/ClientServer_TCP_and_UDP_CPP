cmake_minimum_required(VERSION 4.0)
project(ClientServer)

set(CMAKE_CXX_STANDARD 20)

# Сервер
add_executable(serverTCP server.cpp
        wrappers.cpp
        wrappers.h
        dijkstra.cpp
        dijkstra.h
        interaction.cpp
        interaction.h
        tcpServer.cpp
        tcpServer.h
        udpServer.cpp
        udpServer.h
        validation.cpp
        validation.h
)

# Сервер
add_executable(serverUDP server.cpp
        wrappers.cpp
        wrappers.h
        dijkstra.cpp
        dijkstra.h
        interaction.cpp
        interaction.h
        tcpServer.cpp
        tcpServer.h
        udpServer.cpp
        udpServer.h
        validation.cpp
        validation.h
)

# Клиент1
add_executable(clientTCP client.cpp
        wrappers.cpp
        wrappers.h
        dijkstra.cpp
        dijkstra.h
        interaction.cpp
        interaction.h
        tcpClient.cpp
        tcpClient.h
        udpClient.cpp
        udpClient.h
        validation.cpp
        validation.h
)

# Клиент2
add_executable(clientUDP client.cpp
        wrappers.cpp
        wrappers.h
        dijkstra.cpp
        dijkstra.h
        interaction.cpp
        interaction.h
        tcpClient.cpp
        tcpClient.h
        udpClient.cpp
        udpClient.h
        validation.cpp
        validation.h
)