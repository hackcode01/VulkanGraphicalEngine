#include "input.h"
#include "event.h"
#include "engine_memory.h"
#include "logger.h"

typedef struct KeyboardState {
    b8 keys[256];
} KeyboardState;

typedef struct MouseState {
    i16 x;
    i16 y;

    u8 buttons[BUTTON_MAX_BUTTONS];
} MouseState;

typedef struct InputState {
    KeyboardState keyboardCurrent;
    KeyboardState keyboardPrevious;
    MouseState mouseCurrent;
    MouseState mousePrevious;
} InputState;

/* Internal input state. */
static b8 initialized = FALSE;
static InputState state = {};

void inputInitialize() {
    engineZeroMemory(&state, sizeof(InputState));
    initialized = TRUE;
    ENGINE_INFO("Input subsystem initialized.");
}

void inputShutdown() {
    initialized = FALSE;
}

void inputUpdate(f64 deltaTime) {
    if (!initialized) {
        return;
    }

    /* Copy current states to previous states. */
    engineCopyMemory(&state.keyboardPrevious, &state.keyboardCurrent,
        sizeof(KeyboardState));
    engineCopyMemory(&state.mousePrevious, &state.mouseCurrent,
        sizeof(MouseState));
}

void inputProcessKey(Keys key, b8 pressed) {
    /* Only handle this if the state actually changed. */
    if (state.keyboardCurrent.keys[key] != pressed) {
        /* Update internal state. */
        state.keyboardCurrent.keys[key] = pressed;

        /* Fire off an event for immediate processing. */
        EventContext context;
        context.data.uint16[0] = key;
        eventFire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, context);
    }
}

void inputProcessButton(MouseButtons button, b8 pressed) {
    /* If the state changed, fire an event. */
    if (state.mouseCurrent.buttons[button] != pressed) {
        state.mouseCurrent.buttons[button] = pressed;

        /* Fire the event. */
        EventContext context;
        context.data.uint16[0] = button;
        eventFire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, 0, context);
    }
}

void inputProcessMouseMove(i16 x, i16 y) {
    /* Only process if actually different. */
    if (state.mouseCurrent.x != x || state.mouseCurrent.y != y) {
        /* Update internal state. */
        state.mouseCurrent.x = x;
        state.mouseCurrent.y = y;

        /* Fire the event. */
        EventContext context;
        context.data.uint16[0] = x;
        context.data.uint16[1] = y;

        eventFire(EVENT_CODE_MOUSE_MOVED, 0, context);
    }
}

void inputProcessMouseWheel(i8 z_delta) {
    /* NOTE: no internal state to update. */

    /* Fire the event. */
    EventContext context;
    context.data.uint8[0] = z_delta;
    eventFire(EVENT_CODE_MOUSE_WHEEL, 0, context);
}

b8 inputIsKeyDown(Keys key) {
    if (!initialized) {
        return FALSE;
    }

    return state.keyboardCurrent.keys[key] == TRUE;
}

b8 inputIsKeyUp(Keys key) {
    if (!initialized) {
        return TRUE;
    }

    return state.keyboardCurrent.keys[key] == FALSE;
}

b8 inputWasKeyDown(Keys key) {
    if (!initialized) {
        return FALSE;
    }

    return state.keyboardPrevious.keys[key] == TRUE;
}

b8 inputWasKeyUp(Keys key) {
    if (!initialized) {
        return TRUE;
    }

    return state.keyboardPrevious.keys[key] == FALSE;
}

/* mouse input. */
b8 inputIsButtonDown(MouseButtons button) {
    if (!initialized) {
        return FALSE;
    }

    return state.mouseCurrent.buttons[button] == TRUE;
}

b8 inputIsButtonUp(MouseButtons button) {
    if (!initialized) {
        return TRUE;
    }

    return state.mouseCurrent.buttons[button] == FALSE;
}

b8 inputWasButtonDown(MouseButtons button) {
    if (!initialized) {
        return FALSE;
    }

    return state.mousePrevious.buttons[button] == TRUE;
}

b8 inputWasButtonUp(MouseButtons button) {
    if (!initialized) {
        return TRUE;
    }

    return state.mousePrevious.buttons[button] == FALSE;
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
