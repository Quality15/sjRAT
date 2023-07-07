#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char* argv[])
{
    char ip_addr[] = "192.168.31.135";
    int port = 4444;

    // Startup WinSock
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    WSADATA wsaData;
    WORD DllVersion = MAKEWORD(2, 1);
    if (WSAStartup(DllVersion, &wsaData) != 0)
    {
        printf("WinSock startup failed\n");
        exit(1);
    }

    SOCKADDR_IN addr;
    int sizeofaddr = sizeof(addr);
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // to public connections | inet_addr("127.0.0.1") <- connections only from local network
    // addr.sin_addr.s_addr = inet_addr(ip_addr);
    addr.sin_port = htons(port); // port
    addr.sin_family = AF_INET; // IPv4

    SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL); // socket that listens for connetions
    bind(sListen, (SOCKADDR*)&addr, sizeof(addr)); // bind address to the socket
    printf("[*] Listening for connections...\n");
    listen(sListen, SOMAXCONN); // start listen for connections

    SOCKET client;
    client = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);
    if (client == 0)
    {
        printf("Failed to accept client's connection\n%d\n", WSAGetLastError());
        exit(1);
    } else {
        // Get connected client IP
        struct sockaddr_in addr_in;
        int addrSize = sizeof(addr_in);
        getpeername(client, (struct sockaddr*)&addr_in, &addrSize);
        const char* clientIP = inet_ntoa(addr_in.sin_addr);
        printf("[+] New connection from %s!\n", clientIP);
        
        std::string command;
        while (true) 
        {
            std::cout << "sj> ";
            std::getline(std::cin, command);

            if (command == "msgbox") {
                send(client, command.c_str(), command.size(), 0);
                char buff[1024];
                int bytesReceived = recv(client, buff, sizeof(buff), 0);
                if (bytesReceived > 0) {
                    buff[bytesReceived] = '\0';
                    printf("[*] %s\n", buff);
                }
            }
        }
    }

    system("pause");
}