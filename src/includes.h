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
// #include <opencv2/opencv.hpp>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "gdiplus.lib")

const std::string VERSION =         "1.6.0";