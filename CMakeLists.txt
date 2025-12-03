cmake_minimum_required(VERSION 4.0)
project(ClientServer)

set(CMAKE_CXX_STANDARD 20)

# Сервер
add_executable(server server.cpp
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
)

# Клиент1
add_executable(client1 client.cpp
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
)

# Клиент2
add_executable(client2 client.cpp
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
)