#include "./input.h"
#include "./event.h"
#include "./memory/includes/memory.h"
#include "./logger.h"

typedef struct keyboardState {
    b8 keys_t[256];
} keyboardState_t;

typedef struct mouseState {
    i16 x;
    i16 y;
    u8 buttons_t[BUTTON_MAX_BUTTONS];
} mouseState_t;

typedef struct inputState {
    keyboardState_t keyboardCurrent;
    keyboardState_t keyboardPrevious;
    mouseState_t mouseCurrent;
    mouseState_t mousePrevious;
} inputState_t;

/* Internal input state. */
static b8 initialized = FALSE;
static inputState_t state = {};

void inputInitialize() {
    zeroMemory(&state, sizeof(inputState_t));
    initialized = TRUE;
    INFO("Input subsystem initialized.");
}

void inputShutdown() {
    /* Add shutdown routines when needed. */
    initialized = FALSE;
}

void inputUpdate(f64 deltaTime) {
    if (!initialized) {
        return;
    }

    /* Copy current states to previous states. */
    copyMemory(&state.keyboardPrevious, &state.keyboardCurrent, sizeof(keyboardState_t));
    copyMemory(&state.mousePrevious, &state.mouseCurrent, sizeof(mouseState_t));
}

void inputProcessKey(keys_t key, b8 pressed) {
    /* Only handle this if the state actually changed. */
    if (state.keyboardCurrent.keys_t[key] != pressed) {
        /* Update internal state. */
        state.keyboardCurrent.keys_t[key] = pressed;

        /* Fire off an event for immediate processing. */
        eventContext_t context;
        context.data_t.u16[0] = key;
        eventFire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED,
                  0, context);
    }
}

void inputProcessButton(buttons_t button, b8 pressed) {
    /* If the state changed, fire an event. */
    if (state.mouseCurrent.buttons_t[button] != pressed) {
        state.mouseCurrent.buttons_t[button] = pressed;

        /* Fire the event. */
        eventContext_t context;
        context.data_t.u16[0] = button;
        eventFire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED,
                  0, context);
    }
}

void inputProcessMouseMove(i16 x, i16 y) {
    /* Only process if actually different. */
    if (state.mouseCurrent.x != x || state.mouseCurrent.y != y) {
        /* Enable this if debugging. */

        /* Update internal state. */
        state.mouseCurrent.x = x;
        state.mouseCurrent.y = y;

        /* Fire the event. */
        eventContext_t context;
        context.data_t.u16[0] = x;
        context.data_t.u16[1] = y;
        eventFire(EVENT_CODE_MOUSE_MOVED, 0, context);
    }
}

/* No internal state to update. */
void inputProcessMouseWheel(i8 z_delta) {
    /* Fire the event. */
    eventContext_t context;
    context.data_t.u8[0] = z_delta;
    eventFire(EVENT_CODE_MOUSE_WHEEL, 0, context);
}

b8 inputIsKeyDown(keys_t key) {
    if (!initialized) {
        return FALSE;
    }

    return state.keyboardCurrent.keys_t[key] == TRUE;
}

b8 inputIsKeyUp(keys_t key) {
    if (!initialized) {
        return TRUE;
    }

    return state.keyboardCurrent.keys_t[key] == FALSE;
}

b8 inputWasKeyDown(keys_t key) {
    if (!initialized) {
        return FALSE;
    }

    return state.keyboardPrevious.keys_t[key] == TRUE;
}

b8 inputWasKeyUp(keys_t key) {
    if (!initialized) {
        return TRUE;
    }

    return state.keyboardPrevious.keys_t[key] == FALSE;
}

/* Mouse input. */
b8 inputIsButtonDown(buttons_t button) {
    if (!initialized) {
        return FALSE;
    }

    return state.mouseCurrent.buttons_t[button] == TRUE;
}

b8 inputIsButtonUp(buttons_t button) {
    if (!initialized) {
        return TRUE;
    }

    return state.mouseCurrent.buttons_t[button] == FALSE;
}

b8 inputWasButtonDown(buttons_t button) {
    if (!initialized) {
        return FALSE;
    }

    return state.mousePrevious.buttons_t[button] == TRUE;
}

b8 inputWasButtonUp(buttons_t button) {
    if (!initialized) {
        return TRUE;
    }

    return state.mousePrevious.buttons_t[button] == FALSE;
}

void inputGetMousePosition(i32* x, i32* y) {
    if (!initialized) {
        *x = 0;
        *y = 0;

        return;
    }

    *x = state.mouseCurrent.x;
    *y = state.mouseCurrent.y;
}

void inputGetPreviousMousePosition(i32* x, i32* y) {
    if (!initialized) {
        *x = 0;
        *y = 0;

        return;
    }

    *x = state.mousePrevious.x;
    *y = state.mousePrevious.y;
}
