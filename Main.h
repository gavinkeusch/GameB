#pragma once

#define GAME_NAME       "Game_B"
#define GAME_RES_WIDTH  384
#define GAME_RES_HEIGHT 216
#define GAME_BPP        32
#define GAME_DRAWING_AREA_MEMORY_SIZE (GAME_RES_WIDTH * GAME_RES_HEIGHT * (GAME_BPP / 8))

typedef struct GAMEBITMAP {
    BITMAPINFO bitmapinfo;
    void* memory;
} GAMEBITMAP;

LRESULT CALLBACK MainWindowProc(_In_ HWND windowHandle, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam);
DWORD CreateMainGameWindow(void);
BOOL GameIsAlreadyRunning(void);
void ProcessPlayerInput(void);
void RenderFrameGraphics(void);