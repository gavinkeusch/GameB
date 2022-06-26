#include <stdio.h>
#include <windows.h>
#include "Main.h"

HWND gGameWindow;
BOOL gGameIsRunning;
GAMEBITMAP gDrawingSurface;

int WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR commandLine, int showCommand) {
	UNREFERENCED_PARAMETER(previousInstance);
	UNREFERENCED_PARAMETER(commandLine);
	UNREFERENCED_PARAMETER(showCommand);

    if (GameIsAlreadyRunning() == TRUE) {
        MessageBoxA(NULL, "Another instance of this program is running!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if (CreateMainGameWindow() != ERROR_SUCCESS) {
        goto Exit;
    }

    gDrawingSurface.bitmapinfo.bmiHeader.biSize = sizeof(gDrawingSurface.bitmapinfo.bmiHeader);
    gDrawingSurface.bitmapinfo.bmiHeader.biWidth = GAME_RES_WIDTH;
    gDrawingSurface.bitmapinfo.bmiHeader.biHeight = GAME_RES_HEIGHT;
    gDrawingSurface.bitmapinfo.bmiHeader.biBitCount = GAME_BPP;
    gDrawingSurface.bitmapinfo.bmiHeader.biCompression = BI_RGB;
    gDrawingSurface.bitmapinfo.bmiHeader.biPlanes = 1;
    gDrawingSurface.memory = VirtualAlloc(NULL, GAME_DRAWING_AREA_MEMORY_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (gDrawingSurface.memory == NULL) {
        MessageBoxA(NULL, "Failed to allocate memory for drawing surface!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    MSG message = { 0 };
    gGameIsRunning = TRUE;

    while (gGameIsRunning) {
        while (PeekMessageA(&message, gGameWindow, 0, 0, PM_REMOVE)) {
            DispatchMessageA(&message);
        }

        ProcessPlayerInput();
        RenderFrameGraphics();

        Sleep(1);
    }

Exit:
    return 0;
}

LRESULT CALLBACK MainWindowProc(_In_ HWND windowHandle, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam) {
    LRESULT result = 0;

    switch (message) {
        case WM_CLOSE:
            gGameIsRunning = FALSE;
            PostQuitMessage(0);
            break;

        default:
            result = DefWindowProcA(windowHandle, message, wParam, lParam);
    }

    return result;
}

DWORD CreateMainGameWindow(void) {
    DWORD result = ERROR_SUCCESS;
    WNDCLASSEXA windowClass = { 0 };

    windowClass.cbSize = sizeof(WNDCLASSEXA);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = (WNDPROC) MainWindowProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = GetModuleHandleA(NULL);
    windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    windowClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = GetStockObject(WHITE_BRUSH);
    windowClass.lpszMenuName =  NULL;
    windowClass.lpszClassName = GAME_NAME "_WindowClass";

    if (!RegisterClassExA(&windowClass)) {
        result = GetLastError();

        MessageBox(NULL, "Window Registration Failed!", "Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    gGameWindow = CreateWindowExA(WS_EX_CLIENTEDGE, windowClass.lpszClassName, "Game B", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                   CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, windowClass.hInstance, NULL);

    if (!gGameWindow) {
        result = GetLastError();
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

Exit:
    return result;
}

BOOL GameIsAlreadyRunning(void) {
    CreateMutexA(NULL, FALSE, GAME_NAME "_GameMutex");

    return (GetLastError() == ERROR_ALREADY_EXISTS);
}

void ProcessPlayerInput(void) {
    short escapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);

    if (escapeKeyIsDown) {
        SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
    }
}

void RenderFrameGraphics(void) {

}