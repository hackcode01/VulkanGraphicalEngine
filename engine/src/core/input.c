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
static InputState *statePtr;

void inputSystemInitialize(u64 *memoryRequirement, void *state) {
    *memoryRequirement = sizeof(InputState);
    if (state == 0) {
        return;
    }

    engineZeroMemory(state, sizeof(InputState));
    statePtr = state;
}

void inputSystemShutdown(void *state) {
    statePtr = 0;
}

void inputUpdate(f64 deltaTime) {
    if (!statePtr) {
        return;
    }

    /* Copy current states to previous states. */
    engineCopyMemory(&statePtr->keyboardPrevious, &statePtr->keyboardCurrent,
        sizeof(KeyboardState));
    engineCopyMemory(&statePtr->mousePrevious, &statePtr->mouseCurrent,
        sizeof(MouseState));
}

void inputProcessKey(Keys key, b8 pressed) {
    if (key == KEY_LALT) {
        ENGINE_INFO("Left alt pressed.")
    } else if (key == KEY_RALT) {
        ENGINE_INFO("Right alt pressed.")
    }

    if (key == KEY_LCONTROL) {
        ENGINE_INFO("Left ctrl pressed.")
    } else if (key == KEY_RCONTROL) {
        ENGINE_INFO("Right ctrl pressed.")
    }

    if (key == KEY_LSHIFT) {
        ENGINE_INFO("Left shift pressed.")
    } else if (key == KEY_RSHIFT) {
        ENGINE_INFO("Right shift pressed.")
    }

    /** Only handle this if the state actually changed. */
    if (statePtr->keyboardCurrent.keys[key] != pressed) {
        /* Update internal state. */
        statePtr->keyboardCurrent.keys[key] = pressed;

        /* Fire off an event for immediate processing. */
        EventContext context;
        context.data.uint16[0] = key;
        eventFire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, context);
    }
}

void inputProcessButton(MouseButtons button, b8 pressed) {
    /* If the state changed, fire an event. */
    if (statePtr->mouseCurrent.buttons[button] != pressed) {
        statePtr->mouseCurrent.buttons[button] = pressed;

        /* Fire the event. */
        EventContext context;
        context.data.uint16[0] = button;
        eventFire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, 0, context);
    }
}

void inputProcessMouseMove(i16 x, i16 y) {
    /* Only process if actually different. */
    if (statePtr->mouseCurrent.x != x || statePtr->mouseCurrent.y != y) {
        /* Update internal statePtr-> */
        statePtr->mouseCurrent.x = x;
        statePtr->mouseCurrent.y = y;

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
    if (!statePtr) {
        return false;
    }

    return statePtr->keyboardCurrent.keys[key] == true;
}

b8 inputIsKeyUp(Keys key) {
    if (!statePtr) {
        return true;
    }

    return statePtr->keyboardCurrent.keys[key] == false;
}

b8 inputWasKeyDown(Keys key) {
    if (!statePtr) {
        return false;
    }

    return statePtr->keyboardPrevious.keys[key] == true;
}

b8 inputWasKeyUp(Keys key) {
    if (!statePtr) {
        return true;
    }

    return statePtr->keyboardPrevious.keys[key] == false;
}

/* mouse input. */
b8 inputIsMouseButtonDown(MouseButtons button) {
    if (!statePtr) {
        return false;
    }

    return statePtr->mouseCurrent.buttons[button] == true;
}

b8 inputIsMouseButtonUp(MouseButtons button) {
    if (!statePtr) {
        return true;
    }

    return statePtr->mouseCurrent.buttons[button] == false;
}

b8 inputWasMouseButtonDown(MouseButtons button) {
    if (!statePtr) {
        return false;
    }

    return statePtr->mousePrevious.buttons[button] == true;
}

b8 inputWasMouseButtonUp(MouseButtons button) {
    if (!statePtr) {
        return true;
    }

    return statePtr->mousePrevious.buttons[button] == false;
}

void inputGetMousePosition(i32* x, i32* y) {
    if (!statePtr) {
        *x = 0;
        *y = 0;

        return;
    }

    *x = statePtr->mouseCurrent.x;
    *y = statePtr->mouseCurrent.y;
}

void inputGetPreviousMousePosition(i32* x, i32* y) {
    if (!statePtr) {
        *x = 0;
        *y = 0;

        return;
    }

    *x = statePtr->mousePrevious.x;
    *y = statePtr->mousePrevious.y;
}
