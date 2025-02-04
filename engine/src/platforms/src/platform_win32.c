#include "../includes/platform.h"

#if PLATFORM_WINDOWS

#include "../../core/logger.h"
#include "../../core/input.h"

#include <Windows.h>
#include <windowsx.h>
#include <stdlib.h>

typedef struct {
    HINSTANCE hInstance;
    HWND hwnd;
} internalState_t;

/* Clock. */
static f64 clockFrequency;
static LARGE_INTEGER startTime;

LRESULT CALLBACK win32ProcessMessage(HWND hwnd, u32 message,
                                     WPARAM wParam, LPARAM lParam);

b8 platformStartup(platformState_t* platformState,
                       const char* applicationName,
                       i32 x, i32 y, i32 width, i32 height) {
    platformState->internalState = malloc(sizeof(internalState_t));
    internalState_t* state = (internalState_t*)platformState->internalState;

    state->hInstance = GetModuleHandleA(0);

    /* Setup and register window class. */
    HICON hIcon = LoadIcon(state->hInstance, IDI_APPLICATION);
    WNDCLASSA wndClassA;
    memset(&wndClassA, 0, sizeof(wndClassA));

    /* Get double-clicks. */
    wndClassA.style = CS_DBLCLKS;
    wndClassA.lpfnWndProc = win32ProcessMessage;
    wndClassA.cbClsExtra = 0;
    wndClassA.cbWndExtra = 0;
    wndClassA.hInstance = state->hInstance;
    wndClassA.hIcon = hIcon;
    wndClassA.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClassA.hbrBackground = NULL;
    wndClassA.lpszClassName = "GraphicalEngineWindowClass";

    if (!RegisterClassA(&wndClassA)) {
        MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    /* Create window. */
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
    RECT borderRectangle = { 0, 0, 0, 0 };
    AdjustWindowRectEx(&borderRectangle, windowStyle, 0, windowExStyle);

    /* In this case, the border rectangle is negative. */
    windowX += borderRectangle.left;
    windowY += borderRectangle.top;

    /* Grow by the size of the OS border. */
    windowWidth += borderRectangle.right - borderRectangle.left;
    windowHeight += borderRectangle.bottom - borderRectangle.top;

    HWND handle = CreateWindowExA(windowExStyle, "GraphicalEngineWindowClass",
                                  applicationName, windowStyle, windowX, windowY,
                                  windowWidth, windowHeight,
                                  0, 0, state->hInstance, 0);

    if (handle == 0) {
        MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        FATAL("Window creation failed!");

        return FALSE;
    } else {
        state->hwnd = handle;
    }

    /* Show the window. */

    /* if the window should not accept input, this should be false. */
    b32 shouldActivate = 1;
    i32 showWindowCommandFlags = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
    
    /* If initially minimized, use SW_MINIMIZE : SW_SHOWMINNOACTIVE;
     * If initially maximized, use SW_SHOWMAXIMIZED : SW_MAXIMIZE
     */
    ShowWindow(state->hwnd, showWindowCommandFlags);

    /* Clock setup. */
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clockFrequency = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&startTime);

    return TRUE;
}

void platformShutdown(platformState_t* platformState) {
    internalState_t* state = (internalState_t*)platformState->internalState;

    if (state->hwnd) {
        DestroyWindow(state->hwnd);
        state->hwnd = 0;
    }
}

b8 platformPumpMessages(platformState_t* platformState) {
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

void platformFree(void *block, b8 aligned) {
    free(block);
}

void* platformZeroMemory(void *block, u64 size) {
    return memset(block, 0, size);
}

void* platformCopyMemory(void *destination, const void *source, u64 size) {
    return memcpy(destination, source, size);
}

void* platformSetMemory(void *destination, i32 value, u64 size) {
    return memset(destination, value, size);
}

void platformConsoleWrite(const char *message, u8 colour) {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    /* FATAL,ERROR,WARN,INFO,DEBUG,TRACE. */
    static u8 levels[6] = { 64, 4, 6, 2, 1, 8 };
    SetConsoleTextAttribute(consoleHandle, levels[colour]);
    OutputDebugStringA(message);

    u64 length = strlen(message);
    LPDWORD numberWritten = 0;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length,
                  numberWritten, 0);
}

void platformConsoleWriteError(const char *message, u8 colour) {
    HANDLE consoleHandle = GetStdHandle(STD_ERROR_HANDLE);

    /* FATAL,ERROR,WARN,INFO,DEBUG,TRACE. */
    static u8 levels[6] = { 64, 4, 6, 2, 1, 8 };
    SetConsoleTextAttribute(consoleHandle, levels[colour]);
    OutputDebugStringA(message);

    u64 length = strlen(message);
    LPDWORD numberWritten = 0;
    WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length,
                  numberWritten, 0);
}

f64 platformGetAbsoluteTime() {
    LARGE_INTEGER nowTime;
    QueryPerformanceCounter(&nowTime);

    return (f64)nowTime.QuadPart * clockFrequency;
}

void platformSleep(u64 ms) {
    Sleep(ms);
}

LRESULT CALLBACK win32ProcessMessage(HWND hwnd, u32 message,
                                     WPARAM wParam, LPARAM lParam) {
    switch (message) {
        /* Notify the OS that erasing will be handled by the application to prevent flicker. */
        case WM_ERASEBKGND:
            return 1;

        /* Fire an event for the application to quit. */
        case WM_CLOSE:
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_SIZE: {} break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            b8 pressed = (message == WM_KEYDOWN || message == WM_SYSKEYDOWN);
            keys_t key = (u16)wParam;

            /* Pass to the input subsystem for processing. */
            inputProcessKey(key, pressed);
        } break;
        case WM_MOUSEMOVE: {
            i32 positionX = GET_X_LPARAM(lParam);
            i32 positionY = GET_Y_LPARAM(lParam);

            /* Pass over to the input subsystem. */
            inputProcessMouseMove(positionX, positionY);
        } break;
        case WM_MOUSEWHEEL: {
            i32 deltaZ = GET_WHEEL_DELTA_WPARAM(wParam);
            if (deltaZ != 0) {
                /* Flatten the input to an OS-independent (-1, 1). */
                deltaZ = (deltaZ < 0) ? -1 : 1;
                inputProcessMouseWheel(deltaZ);
            }
        } break;

        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            b8 pressed = message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN || message == WM_MBUTTONDOWN;
            buttons_t mouseButton = BUTTON_MAX_BUTTONS;
            switch (message) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                    mouseButton = BUTTON_LEFT;
                    break;
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                    mouseButton = BUTTON_MIDDLE;
                    break;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                    mouseButton = BUTTON_RIGHT;
                    break;
            }

            /* Pass over to the input subsystem. */
            if (mouseButton != BUTTON_MAX_BUTTONS) {
                inputProcessButton(mouseButton, pressed);
            }
        } break;
    }

    return DefWindowProcA(hwnd, message, wParam, lParam);
}

#endif
