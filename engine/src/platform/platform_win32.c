#include "platform.h"

/* Windows platform layer. */
#if PLATFORM_WINDOWS

#include "../core/logger.h"

#include <Windows.h>
#include <windowsx.h>
#include <stdlib.h>

typedef struct {
    HINSTANCE hInstance;
    HWND hwnd;
} InternalState_t;

/* Variables for Clock. */
static f64 clockFrequency;
static LARGE_INTEGER startTime;

LRESULT CALLBACK win32ProcessMessage(HWND hwnd, u32 message, WPARAM wParam, LPARAM lParam);

b8 platformStartup(
    PlatformState_t *platformState,
    const char* applicationName,
    i32 x,
    i32 y,
    i32 width,
    i32 height) {
    platformState->internalState = malloc(sizeof(InternalState_t));
    InternalState_t* state = (InternalState_t *)platformState->internalState;

    state->hInstance = GetModuleHandleA(0);

    /* Setup and register window class. */
    HICON icon = LoadIcon(state->hInstance, IDI_APPLICATION);
    WNDCLASSA wndClassA;
    memset(&wndClassA, 0, sizeof(wndClassA));
    wndClassA.style = CS_DBLCLKS;
    wndClassA.lpfnWndProc = win32ProcessMessage;
    wndClassA.cbClsExtra = 0;
    wndClassA.cbWndExtra = 0;
    wndClassA.hInstance = state->hInstance;
    wndClassA.hIcon = icon;
    wndClassA.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClassA.hbrBackground = NULL;
    wndClassA.lpszClassName = "EngineWindowClass";

    if (!RegisterClassA(&wndClassA)) {
        MessageBoxA(0, "Window registration failed", "Error",
                    MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    /* Create Window. */
    u32 clientX = x;
    u32 clientY = y;
    u32 clientWidth = width;
    u32 clientHeight = height;

    u32 windowX = clientX;
    u32 windowY = clientY;
    u32 windowWidth = clientWidth;
    u32 windowHeight = clientHeight;

    u32 windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    u32 windowExStyle = WS_EX_APPWINDOW;

    windowStyle |= WS_MAXIMIZEBOX;
    windowStyle |= WS_MINIMIZEBOX;
    windowStyle |= WS_THICKFRAME;

    /* Obtain the size of the border. */
    RECT borderRectangle = {0, 0, 0, 0};
    AdjustWindowRectEx(&borderRectangle, windowStyle, 0, windowExStyle);

    /* In this case, the border rectangle is negative. */
    windowX += borderRectangle.left;
    windowY += borderRectangle.top;

    /* Grow by the size of the OS border. */
    windowWidth += borderRectangle.right - borderRectangle.left;
    windowHeight += borderRectangle.bottom - borderRectangle.top;

    HWND handle = CreateWindowExA(
        windowExStyle, "EngineWindowClass", applicationName,
        windowStyle, windowX, windowY, windowWidth, windowHeight,
        0, 0, state->hInstance, 0);

    if (handle == 0) {
        MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        ENGINE_FATAL("Window creation failed!");

        return FALSE;
    } else {
        state->hwnd = handle;
    }

    /* Show the window. */
    b32 shouldActivate = 1;
    i32 showWindowCommandFlags = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;

    /** If initially minimized, use SW_MINIMIZE : SW_SHOWMINNOACTIVE;
     *  If initially maximized, use SW_SHOWMAXIMIZED : SW_MAXIMIZE
     */
    ShowWindow(state->hwnd, showWindowCommandFlags);

    /* Clock setup. */
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    clockFrequency = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&startTime);

    return TRUE;
}

void platformShutdown(PlatformState_t* platformState) {
    /* Simply cold-cast to the known type. */
    InternalState_t *state = (InternalState_t *)platformState->internalState;

    if (state->hwnd) {
        DestroyWindow(state->hwnd);
        state->hwnd = 0;
    }
}

b8 platformPumpMessages(PlatformState_t* platformState) {
    MSG message;

    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return TRUE;
}

void* platformAllocate(u64 size, b8 aligned) {
    return malloc(size);
}

void platformFree(void* block) {
    free(block);
}

void* platformZeroMemory(void* block, u64 size) {
    return memset(block, 0, size);
}

void* platformCopyMemory(void* dest, const void* source, u64 size) {
    return memcpy(dest, source, size);
}

void* platformSetMemory(void* dest, i32 value, u64 size) {
    return memset(dest, value, size);
}

void platformConsoleWrite(const char* message, u8 color) {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    /* FATAL, ERROR, WARNING, INFO, DEBUG, TRACE */
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(consoleHandle, levels[color]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD numberWritten = 0;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, numberWritten, 0);
}

void platformConsoleWriteError(const char* message, u8 color) {
    HANDLE consoleHandle = GetStdHandle(STD_ERROR_HANDLE);

    /* FATAL, ERROR, WARNING, INFO, DEBUG, TRACE */
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(consoleHandle, levels[color]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD numberWritten = 0;
    WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, numberWritten, 0);
}

f64 platformGetAbsoluteTime() {
    LARGE_INTEGER nowTime;
    QueryPerformanceCounter(&nowTime);
    return (f64)nowTime.QuadPart * clockFrequency;
}

void platformSleep(u64 ms) {
    Sleep(ms);
}

LRESULT CALLBACK win32ProcessMessage(HWND hwnd, u32 message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_ERASEBKGND:
            /* Notify the OS that erasing will be handled by the application to prevent flicker. */
            return 1;
        case WM_CLOSE:
            /* Fire an event for the application to quit. */
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {} break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {} break;
        case WM_MOUSEMOVE: {} break;
        case WM_MOUSEWHEEL: {} break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {} break;
    }

    return DefWindowProcA(hwnd, message, wParam, lParam);
}

#endif /* PLATFORM_WINDOWS */
