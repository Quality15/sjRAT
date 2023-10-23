// widgets.h
// define of all variables (for widnows.cpp)

#pragma once

// #include <Windows.h>
// #include <WinSock2.h>
// #include <WS2tcpip.h>
#include "includes.h"

#define OnMenuExitClick         1

void MainWndMenus(HWND hWnd);
void MainWndWidgets(HWND hWnd);

HWND hStaticTest;
bool isVisible = true;