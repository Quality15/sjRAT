#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <mutex>
#include <fstream>
#include <filesystem>
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
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "gdiplus.lib")

const std::string VERSION =         "2.2";

#define OPEN_SCREENSHOT         1     // open screenshot after saving (on server)
#define CHECK_VERSION           1     // check version from github (server)
#define OPEN_URL                0     // open gh page on run (server)
#define MSGBOX_SHOW             0     // show msgbox on connection (client)
#define ELEVATION_MSG           0     // show msgbox if run without admin rights (client)