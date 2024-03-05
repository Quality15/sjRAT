// window.cpp
// main file that will makes GUI window

#include "widgets.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
TCHAR WinName[] = _T("MainFrame");
int APIENTRY _tWinMain(HINSTANCE This,
	HINSTANCE Prev,
	LPTSTR cmd,
	int mode)
{
	HWND hWnd;
	MSG msg;
	WNDCLASS wc;
	wc.hInstance = This;
	wc.lpszClassName = WinName;
	wc.lpfnWndProc = WndProc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	if (!RegisterClass(&wc)) return 0;

	hWnd = CreateWindow(WinName,
		"sjRAT Limited",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		1280,
		720,
		HWND_DESKTOP,
		NULL,
		This,
		NULL);
	ShowWindow(hWnd, mode);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
        FreeConsole(); // hide console
		ShowWindow(GetConsoleWindow(), SW_HIDE); // hide console;
        MainWndMenus(hWnd);
        MainWndWidgets(hWnd);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_SPACE) {
			if (isVisible) {
				ShowWindow(hStaticTest, SW_HIDE); // hide
				isVisible = false; 
			} else {
				ShowWindow(hStaticTest, SW_SHOW); // show
				isVisible = true; 
			}
		}

	case WM_COMMAND:
		switch (wParam)
		{
            case OnMenuExitClick:
                PostQuitMessage(0);
                break;
        }
		break;
	case WM_DESTROY: PostQuitMessage(0);
		break;

	default: return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void MainWndMenus(HWND hWnd) // Add menu into window 
{
    HMENU RootMenu = CreateMenu();
    HMENU SubMenu = CreateMenu();

    AppendMenu(SubMenu, MF_STRING, NULL, "test");
    AppendMenu(SubMenu, MF_STRING, NULL, "test");
    AppendMenu(SubMenu, MF_STRING, NULL, "test");

    AppendMenu(RootMenu, MF_POPUP, (UINT_PTR)SubMenu, "File");

    SetMenu(hWnd, RootMenu);
}

void MainWndWidgets(HWND hWnd)
{
    hStaticTest = CreateWindow("static", "Test",
        WS_VISIBLE | WS_CHILD,
        10, 10, // x, y
        100, 30, // w, h
        hWnd, NULL, NULL, NULL);
}