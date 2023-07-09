#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <mutex>
#include <fstream>
#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <chrono>
#include <mmsystem.h>
#include <tlhelp32.h>
#include <ctime>
#include <gdiplus.h>
#include <random>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "gdiplus.lib")

#define FILE_BUFF_SIZE              100'000'000

void ReceiveFile(SOCKET s, const std::string& filename);
void SendFile(SOCKET s, char* filename);