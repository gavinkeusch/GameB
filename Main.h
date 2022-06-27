#pragma once

#define GAME_NAME       "Game_B"
#define GAME_RES_WIDTH  384
#define GAME_RES_HEIGHT 240
#define GAME_BPP        32
#define GAME_DRAWING_AREA_MEMORY_SIZE (GAME_RES_WIDTH * GAME_RES_HEIGHT * (GAME_BPP / 8))
#define CALCULATE_AVG_FPS_EVERY_X_FRAMES 100
#define TARGET_MICROSECONDS_PER_FRAME 16667

typedef LONG(NTAPI* _NtQueryTimerResolution) (OUT PULONG MinimumResolution, OUT PULONG MaximumResolution, OUT PULONG CurrentResolution);
_NtQueryTimerResolution NtQueryTimerResolution;

typedef struct GAMEBITMAP {
    BITMAPINFO bitmapinfo;
    void* memory;
} GAMEBITMAP;

typedef struct PIXEL32 {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} PIXEL32;

typedef struct PERFORMANCEDATA {
    uint64_t totalFramesRendered;
    float rawFPSAverage;
    float cookedFPSAverage;
    int64_t perfFrequency;
    MONITORINFO monitorInfo;
    int32_t monitorWidth;
    int32_t monitorHeight;
    BOOL displayDebugInfo;
    LONG minimumTimerResolution;
    LONG maximumnTimerResolution;
    LONG currentTimerResolution;
} PERFORMANCEDATA;

LRESULT CALLBACK MainWindowProc(_In_ HWND windowHandle, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam);
DWORD CreateMainGameWindow(void);
BOOL GameIsAlreadyRunning(void);
void ProcessPlayerInput(void);
void RenderFrameGraphics(void);
void ClearScreen(_In_ __m128i color);