#include "platform.h"

/* Linux platform layer. */
#if PLATFORM_LINUX

#include "../core/logger.h"

#include <xcb/xcb.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h> /* sudo apt-get install libx11-dev */
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h> /* sudo apt-get install libxkbcommon-x11-dev */
#include <sys/time.h>

#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct InternalState {
    Display* display;
    xcb_connection_t* connection;
    xcb_window_t window;
    xcb_screen_t* screen;
    xcb_atom_t wmProtocols;
    xcb_atom_t wmDeleteWindow;
} InternalState;

b8 platformStartup(
    PlatformState_t* platformState,
    const char* applicationName,
    i32 x,
    i32 y,
    i32 width,
    i32 height) {
    /* Create the internal state. */
    platformState->internalState = malloc(sizeof(InternalState));
    InternalState* state = (InternalState*)platformState->internalState;

    /* Connect to X. */
    state->display = XOpenDisplay(NULL);

    /* Turn off key repeats. */
    XAutoRepeatOff(state->display);

    /* Retrieve the connection from the display. */
    state->connection = XGetXCBConnection(state->display);

    if (xcb_connection_has_error(state->connection)) {
        ENGINE_FATAL("Failed to connect to X server via XCB.")
        return FALSE;
    }

    /* Get data from the X server */
    const struct xcb_setup_t* setup = xcb_get_setup(state->connection);

    /* Loop through screens using iterator. */
    xcb_screen_iterator_t iterator = xcb_setup_roots_iterator(setup);
    int screen_p = 0;
    for (i32 s = screen_p; s > 0; s--) {
        xcb_screen_next(&iterator);
    }

    /* After screens have been looped through, assign iterator. */
    state->screen = iterator.data;

    /* Allocate a XID for the window to be created. */
    state->window = xcb_generate_id(state->connection);

    /** Register event types.
     * XCB_CW_BACK_PIXEL = filling then window bg with a single colour
     * XCB_CW_EVENT_MASK is required.
     */
    u32 eventMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

    /* Listen for keyboard and mouse buttons. */
    u32 eventValues = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
                       XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
                       XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION |
                       XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    /* Values to be sent over XCB (bg colour, events). */
    u32 valueList[] = {state->screen->black_pixel, eventValues};

    /* Create the window. */
    xcb_void_cookie_t cookie = xcb_create_window(
        state->connection,
        XCB_COPY_FROM_PARENT,
        state->window,
        state->screen->root,
        x,
        y,
        width,
        height,
        0,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        state->screen->root_visual,
        eventMask,
        valueList
    );

    /* Change the title. */
    xcb_change_property(
        state->connection,
        XCB_PROP_MODE_REPLACE,
        state->window,
        XCB_ATOM_WM_NAME,
        XCB_ATOM_STRING,
        8,
        strlen(applicationName),
        applicationName);

    /**
     * Tell the server to notify when the window manager
     * attempts to destroy the window.
     */
    xcb_intern_atom_cookie_t wmDeleteCookie = xcb_intern_atom(
        state->connection,
        0,
        strlen("WM_DELETE_WINDOW"),
        "WM_DELETE_WINDOW");
    xcb_intern_atom_cookie_t wmProtocolsCookie = xcb_intern_atom(
        state->connection,
        0,
        strlen("WM_PROTOCOLS"),
        "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* wmDeleteReply = xcb_intern_atom_reply(
        state->connection,
        wmDeleteCookie,
        NULL);
    xcb_intern_atom_reply_t* wmProtocolsReply = xcb_intern_atom_reply(
        state->connection,
        wmProtocolsCookie,
        NULL);
    state->wmDeleteWindow = wmDeleteReply->atom;
    state->wmProtocols = wmProtocolsReply->atom;

    xcb_change_property(
        state->connection,
        XCB_PROP_MODE_REPLACE,
        state->window,
        wmProtocolsReply->atom,
        4,
        32,
        1,
        &wmDeleteReply->atom);

    /* Map the window to the screen. */
    xcb_map_window(state->connection, state->window);

    /* Flush the stream. */
    i32 stream_result = xcb_flush(state->connection);
    if (stream_result <= 0) {
        ENGINE_FATAL("An error occurred when flusing the stream: %d", stream_result)
        return FALSE;
    }

    return TRUE;
}

void platformShutdown(PlatformState_t* platformState) {
    /* Simply cold-cast to the known type. */
    InternalState* state = (InternalState*)platformState->internalState;

    /* Turn key repeats back on since this is global for the OS... just... wow. */
    XAutoRepeatOn(state->display);

    xcb_destroy_window(state->connection, state->window);
}

b8 platformPumpMessages(PlatformState_t* platformState) {
    /* Simply cold-cast to the known type. */
    InternalState* state = (InternalState*)platformState->internalState;

    xcb_generic_event_t* event;
    xcb_client_message_event_t* clientMessageEvent;

    b8 quitFlagged = FALSE;

    /* Poll for events until null is returned. */
    while (event != 0) {
        event = xcb_poll_for_event(state->connection);
        if (event == 0) {
            break;
        }

        /* Input events */
        switch (event->response_type & ~0x80) {
            case XCB_KEY_PRESS:
            case XCB_KEY_RELEASE: {} break;
            case XCB_BUTTON_PRESS:
            case XCB_BUTTON_RELEASE: {} break;
            case XCB_MOTION_NOTIFY: {} break;
            case XCB_CONFIGURE_NOTIFY: {} break;

            case XCB_CLIENT_MESSAGE: {
                clientMessageEvent = (xcb_client_message_event_t*)event;

                /* Window close. */
                if (clientMessageEvent->data.data32[0] == state->wmDeleteWindow) {
                    quitFlagged = TRUE;
                }
            } break;
            default:
                break;
        }

        free(event);
    }
    return !quitFlagged;
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

void platformConsoleWrite(const char* message, u8 colour) {
    /* FATAL, ERROR, WARNING, INFO, DEBUG, TRACE */
    const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
    printf("\033[%sm%s\033[0m", colour_strings[colour], message);
}

void platformConsoleWriteError(const char* message, u8 colour) {
    /* FATAL, ERROR, WARNING, INFO, DEBUG, TRACE */
    const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
    printf("\033[%sm%s\033[0m", colour_strings[colour], message);
}

f64 platformGetAbsoluteTime() {
    struct timespec now;
    clock_gettime(0, &now);
    return now.tv_sec + now.tv_nsec * 0.000000001;
}

void platformSleep(u64 ms) {
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000 * 1000;
    nanosleep(&ts, 0);
#else
    if (ms >= 1000) {
        sleep(ms / 1000);
    }
    usleep((ms % 1000) * 1000);
#endif
}

#endif
