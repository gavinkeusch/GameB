#include <stdio.h>
#include <windows.h>
#include <stdint.h>
#include "Main.h"

HWND gGameWindow;
BOOL gGameIsRunning;
GAMEBITMAP gBackBuffer;
PERFORMANCEDATA gPerformanceData;

int WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR commandLine, int showCommand) {
	UNREFERENCED_PARAMETER(previousInstance);
	UNREFERENCED_PARAMETER(commandLine);
	UNREFERENCED_PARAMETER(showCommand);

    MSG message = { 0 };
    int64_t frameStart = 0;
    int64_t frameEnd = 0;
    int64_t elapsedMicroseconds;
    int64_t elapsedMicrosecondsAccumulatorRaw = 0;
    int64_t elapsedMicrosecondsAccumulatorCooked = 0;

    if (GameIsAlreadyRunning() == TRUE) {
        MessageBoxA(NULL, "Another instance of this program is running!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if (CreateMainGameWindow() != ERROR_SUCCESS) {
        goto Exit;
    }

    QueryPerformanceFrequency((LARGE_INTEGER*) &gPerformanceData.perfFrequency);

    gBackBuffer.bitmapinfo.bmiHeader.biSize = sizeof(gBackBuffer.bitmapinfo.bmiHeader);
    gBackBuffer.bitmapinfo.bmiHeader.biWidth = GAME_RES_WIDTH;
    gBackBuffer.bitmapinfo.bmiHeader.biHeight = GAME_RES_HEIGHT;
    gBackBuffer.bitmapinfo.bmiHeader.biBitCount = GAME_BPP;
    gBackBuffer.bitmapinfo.bmiHeader.biCompression = BI_RGB;
    gBackBuffer.bitmapinfo.bmiHeader.biPlanes = 1;
    gBackBuffer.memory = VirtualAlloc(NULL, GAME_DRAWING_AREA_MEMORY_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (gBackBuffer.memory == NULL) {
        MessageBoxA(NULL, "Failed to allocate memory for drawing surface!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    gGameIsRunning = TRUE;

    while (gGameIsRunning) {
        QueryPerformanceCounter((LARGE_INTEGER*) &frameStart);

        while (PeekMessageA(&message, gGameWindow, 0, 0, PM_REMOVE)) {
            DispatchMessageA(&message);
        }

        ProcessPlayerInput();
        RenderFrameGraphics();

        QueryPerformanceCounter((LARGE_INTEGER*)&frameEnd);

        elapsedMicroseconds = frameEnd - frameStart;
        elapsedMicroseconds *= 1000000;
        elapsedMicroseconds /= gPerformanceData.perfFrequency;

        gPerformanceData.totalFramesRendered++;
        elapsedMicrosecondsAccumulatorRaw += elapsedMicroseconds;

        while (elapsedMicroseconds <= TARGET_MICROSECONDS_PER_FRAME) {
            Sleep(0);

            elapsedMicroseconds = frameEnd - frameStart;
            elapsedMicroseconds *= 1000000;
            elapsedMicroseconds /= gPerformanceData.perfFrequency;

            QueryPerformanceCounter((LARGE_INTEGER*)&frameEnd);
        }

        elapsedMicrosecondsAccumulatorCooked += elapsedMicroseconds;

        if ((gPerformanceData.totalFramesRendered % CALCULATE_AVG_FPS_EVERY_X_FRAMES) == 0) {
            gPerformanceData.rawFPSAverage = 1.0f / (((float)elapsedMicrosecondsAccumulatorRaw / CALCULATE_AVG_FPS_EVERY_X_FRAMES) * 0.000001f);
            gPerformanceData.cookedFPSAverage = 1.0f / (((float)elapsedMicrosecondsAccumulatorCooked / CALCULATE_AVG_FPS_EVERY_X_FRAMES) * 0.000001f);

            elapsedMicrosecondsAccumulatorRaw = 0;
            elapsedMicrosecondsAccumulatorCooked = 0;
        }
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
    WNDCLASSEXA windowClass = {0};

    windowClass.cbSize = sizeof(WNDCLASSEXA);
    windowClass.style = 0;
    windowClass.lpfnWndProc = (WNDPROC) MainWindowProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = GetModuleHandleA(NULL);
    windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    windowClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));
    windowClass.lpszMenuName =  NULL;
    windowClass.lpszClassName = GAME_NAME "_WindowClass";

    if (!RegisterClassExA(&windowClass)) {
        result = GetLastError();

        MessageBox(NULL, "Window Registration Failed!", "Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    gGameWindow = CreateWindowExA(0, windowClass.lpszClassName, "Game B", WS_VISIBLE,
                                   CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, windowClass.hInstance, NULL);

    if (!gGameWindow) {
        result = GetLastError();
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    gPerformanceData.monitorInfo.cbSize = sizeof(MONITORINFO);

    if(GetMonitorInfoA(MonitorFromWindow(gGameWindow, MONITOR_DEFAULTTOPRIMARY), &gPerformanceData.monitorInfo) == 0) {
        result = ERROR_INVALID_MONITOR_HANDLE;
        goto Exit;
    }

    gPerformanceData.monitorWidth = gPerformanceData.monitorInfo.rcMonitor.right - gPerformanceData.monitorInfo.rcMonitor.left;
    gPerformanceData.monitorHeight = gPerformanceData.monitorInfo.rcMonitor.bottom - gPerformanceData.monitorInfo.rcMonitor.top;

    if (SetWindowLongPtrA(gGameWindow, GWL_STYLE, WS_VISIBLE) == 0) {
        result = GetLastError();
        goto Exit;
    }

    if (SetWindowPos(gGameWindow, HWND_TOP, gPerformanceData.monitorInfo.rcMonitor.left, gPerformanceData.monitorInfo.rcMonitor.top,
                     gPerformanceData.monitorWidth, gPerformanceData.monitorHeight, SWP_FRAMECHANGED) == 0) {
        result = GetLastError();
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
    int16_t escapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);
    int16_t debugKeyIsDown = GetAsyncKeyState(VK_F1);
    static int16_t debugKeyWasDown;

    if (escapeKeyIsDown) {
        SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
    }

    if (debugKeyIsDown && !debugKeyWasDown) {
        gPerformanceData.displayDebugInfo = !gPerformanceData.displayDebugInfo;
    }

    debugKeyWasDown = debugKeyIsDown;
}

void RenderFrameGraphics(void) {
    PIXEL32 pixel = {0};
    pixel.blue = 0x7f;
    pixel.green = 0;
    pixel.red = 0;
    pixel.alpha = 0xff;

    for (int i = 0; i < GAME_RES_WIDTH * GAME_RES_HEIGHT; i++) {
        memcpy_s((PIXEL32*)gBackBuffer.memory + i, sizeof(PIXEL32), &pixel, sizeof(PIXEL32));
    }

    int32_t screenX = 25;
    int32_t screenY = 25;
    int32_t startingScreenPixel = ((GAME_RES_WIDTH * GAME_RES_HEIGHT) - GAME_RES_WIDTH) - (GAME_RES_WIDTH * screenY) + screenX;

    for (int32_t y = 0;y < 16; y++) {
        for (int32_t x = 0; x < 16; x++) {
            memset((PIXEL32*)gBackBuffer.memory + (uintptr_t)startingScreenPixel + x - ((uintptr_t)GAME_RES_WIDTH * y), 0xff, sizeof(PIXEL32));
        }
    }

    HDC deviceContext = GetDC(gGameWindow);

    StretchDIBits(deviceContext, 0, 0, gPerformanceData.monitorWidth, gPerformanceData.monitorHeight, 0,0,
                  GAME_RES_WIDTH,GAME_RES_HEIGHT, gBackBuffer.memory, &gBackBuffer.bitmapinfo, DIB_RGB_COLORS, SRCCOPY);

    if (gPerformanceData.displayDebugInfo) {
        SelectObject(deviceContext, (HFONT) GetStockObject(ANSI_FIXED_FONT));
        char DebugTextBuffer[64] = { 0 };
        sprintf_s(DebugTextBuffer, _countof(DebugTextBuffer), "FPS Raw:    %.01f", gPerformanceData.rawFPSAverage);
        TextOutA(deviceContext,10 , 10, DebugTextBuffer, (int)strlen(DebugTextBuffer));
        sprintf_s(DebugTextBuffer, _countof(DebugTextBuffer), "FPS Cooked: %.01f", gPerformanceData.cookedFPSAverage);
        TextOutA(deviceContext,10 , 23, DebugTextBuffer, (int)strlen(DebugTextBuffer));
    }

    ReleaseDC(gGameWindow, deviceContext);
}