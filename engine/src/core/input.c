#include "input.h"
#include "event.h"
#include "../engine_memory/engine_memory.h"
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
static b8 initialized = false;
static InputState state = {};

void inputInitialize() {
    engineZeroMemory(&state, sizeof(InputState));
    initialized = true;
    ENGINE_INFO("Input subsystem initialized.")
}

void inputShutdown() {
    initialized = false;
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
        return false;
    }

    return state.keyboardCurrent.keys[key] == true;
}

b8 inputIsKeyUp(Keys key) {
    if (!initialized) {
        return true;
    }

    return state.keyboardCurrent.keys[key] == false;
}

b8 inputWasKeyDown(Keys key) {
    if (!initialized) {
        return false;
    }

    return state.keyboardPrevious.keys[key] == true;
}

b8 inputWasKeyUp(Keys key) {
    if (!initialized) {
        return true;
    }

    return state.keyboardPrevious.keys[key] == false;
}

/* mouse input. */
b8 inputIsButtonDown(MouseButtons button) {
    if (!initialized) {
        return false;
    }

    return state.mouseCurrent.buttons[button] == true;
}

b8 inputIsButtonUp(MouseButtons button) {
    if (!initialized) {
        return true;
    }

    return state.mouseCurrent.buttons[button] == false;
}

b8 inputWasButtonDown(MouseButtons button) {
    if (!initialized) {
        return false;
    }

    return state.mousePrevious.buttons[button] == true;
}

b8 inputWasButtonUp(MouseButtons button) {
    if (!initialized) {
        return true;
    }

    return state.mousePrevious.buttons[button] == false;
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
