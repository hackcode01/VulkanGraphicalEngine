#include "platform.h"

/* Windows platform layer. */
#if PLATFORM_WINDOWS

#include "../core/logger.h"
#include "../core/input.h"
#include "../core/event.h"

#include "../containers/dynamic_array.h"

#include <Windows.h>
#include <windowsx.h>
#include <stdlib.h>

/** For surface creation. */
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include "../renderer/vulkan/vulkan_types.inl"

typedef struct {
    HINSTANCE hInstance;
    HWND hwnd;
    VkSurfaceKHR surface;
} InternalState;

/* Variables for Clock. */
static f64 clockFrequency;
static LARGE_INTEGER startTime;

LRESULT CALLBACK win32ProcessMessage(HWND hwnd, u32 message, WPARAM wParam, LPARAM lParam);

b8 platformStartup(
    PlatformState *platformState,
    const char* applicationName,
    i32 x,
    i32 y,
    i32 width,
    i32 height) {
    platformState->internalState = malloc(sizeof(InternalState));
    InternalState* state = (InternalState *)platformState->internalState;

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
        ENGINE_FATAL("Window creation failed!")

        return FALSE;
    } else {
        state->hwnd = handle;
    }

    /* Show the window. */
    b32 shouldActivate = 1;
    i32 showWindowCommandFlags = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;

    /**
     * If initially minimized, use SW_MINIMIZE : SW_SHOWMINNOACTIVE;
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

void platformShutdown(PlatformState* platformState) {
    /* Simply cold-cast to the known type. */
    InternalState *state = (InternalState *)platformState->internalState;

    if (state->hwnd) {
        DestroyWindow(state->hwnd);
        state->hwnd = 0;
    }
}

b8 platformPumpMessages(PlatformState* platformState) {
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

void platformFree(void* block, b8 aligned) {
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

void platformGetRequiredExtensionNames(const char*** namesDynamicArray) {
    dynamicArrayPush(*namesDynamicArray, &"VK_KHR_win32_surface")
}

/** Surface creation for vulkan. */
b8 platformCreateVulkanSurface(PlatformState* platformState, VulkanContext* context) {
    /** Simply cold-cast to the known type. */
    InternalState* state = (InternalState*)platformState->internalState;

    VkWin32SurfaceCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    createInfo.hinstance = state->hInstance;
    createInfo.hwnd = state->hwnd;

    VkResult result = vkCreateWin32SurfaceKHR(context->instance, &createInfo,
        context->allocator, &state->surface);

    if (result != VK_SUCCESS) {
        ENGINE_FATAL("Vulkan surface creation failed!")
        return FALSE;
    }

    context->surface = state->surface;
    return TRUE;
}

LRESULT CALLBACK win32ProcessMessage(HWND hwnd, u32 message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_ERASEBKGND:
            /* Notify the OS that erasing will be handled by the application to prevent flicker. */
            return 1;
        case WM_CLOSE:
            /* Fire an event for the application to quit. */
            EventContext data = {};
            eventFire(EVENT_CODE_APPLICATION_QUIT, 0, data);
            return TRUE;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {} break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            /** Key pressed / released. */
            b8 pressed = (message == WM_KEYDOWN || message == WM_SYSKEYDOWN);
            Keys key = (u16)wParam;

            /** Pass to the input subsystem for processing. */
            inputProcessKey(key, pressed);
        } break;
        case WM_MOUSEMOVE: {
            /** Mouse move. */
            i32 positionX = GET_X_LPARAM(lParam);
            i32 positionY = GET_Y_LPARAM(lParam);

            /** Pass over to the input subsystem. */
            inputProcessMouseMove(positionX, positionY);
        } break;
        case WM_MOUSEWHEEL: {
            i32 delta_z = GET_WHEEL_DELTA_WPARAM(wParam);
            if (delta_z != 0) {
                /** Flattern the input to an OS-independent (-1, 1). */
                delta_z = (delta_z < 0) ? -1 : 1;
                inputProcessMouseWheel(delta_z);
            }
        } break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            b8 pressed = message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN ||
                         message == WM_MBUTTONDOWN;
            MouseButtons mouse_button = BUTTON_MAX_BUTTONS;
            switch (message) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                    mouse_button = BUTTON_LEFT;
                    break;
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                    mouse_button = BUTTON_MIDDLE;
                    break;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                    mouse_button = BUTTON_RIGHT;
                    break;
            }

            /** Pass over to the input subsystem. */
            if (mouse_button != BUTTON_MAX_BUTTONS) {
                inputProcessButton(mouse_button, pressed);
            }
        } break;
    }

    return DefWindowProcA(hwnd, message, wParam, lParam);
}

#endif /* PLATFORM_WINDOWS */
