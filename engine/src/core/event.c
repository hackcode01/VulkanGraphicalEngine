#include "event.h"

#include "../engine_memory/engine_memory.h"
#include "../containers/dynamic_array.h"

typedef struct RegisteredEvent {
    void* listener;
    PFN_on_event callback;
} RegisteredEvent;

typedef struct EventCodeEntry {
    RegisteredEvent* events;
} EventCodeEntry;

#define MAX_MESSAGES_CODES 15000

/* State structure. */
typedef struct EventSystemState {
    /* Lookup table for event codes. */
    EventCodeEntry registered[MAX_MESSAGES_CODES];
} EventSystemState;

/**
 * Event system internal state.
 */
static b8 isInitialized = false;
static EventSystemState state;

b8 eventInitialize() {
    if (isInitialized == true) {
        return false;
    }

    isInitialized = false;
    engineZeroMemory(&state, sizeof(state));

    isInitialized = true;

    return true;
}

void eventShutdown() {
    /* Free the events arrays. And objects pointed to should be destroyed on their own. */
    for (u16 i = 0; i < MAX_MESSAGES_CODES; ++i) {
        if (state.registered[i].events != 0) {
            dynamicArrayDestroy(state.registered[i].events);
            state.registered[i].events = 0;
        }
    }
}

b8 eventRegister(u16 code, void* listener, PFN_on_event on_event) {
    if (isInitialized == false) {
        return false;
    }

    if (state.registered[code].events == 0) {
        state.registered[code].events = dynamicArrayCreate(RegisteredEvent);
    }

    u64 registeredCount = dynamicArrayLength(state.registered[code].events);
    for (u64 i = 0; i < registeredCount; ++i) {
        if (state.registered[code].events[i].listener == listener) {
            return false;
        }
    }

    /* If at this point, no duplicate was found. Proceed with registration. */
    RegisteredEvent event;
    event.listener = listener;
    event.callback = on_event;
    dynamicArrayPush(state.registered[code].events, event);

    return true;
}

b8 eventUnregister(u16 code, void* listener, PFN_on_event on_event) {
    if (isInitialized == false) {
        return false;
    }

    /* On nothing is registered for the code, boot out. */
    if (state.registered[code].events == 0) {
        return false;
    }

    u64 registeredCount = dynamicArrayLength(state.registered[code].events);
    for (u64 i = 0; i < registeredCount; ++i) {
        RegisteredEvent event = state.registered[code].events[i];

        if (event.listener == listener && event.callback == on_event) {
            /* Found one, remove it. */
            RegisteredEvent popped_event;
            dynamicArrayPopAt(state.registered[code].events, i, &popped_event);
            return true;
        }
    }

    /* Not found. */
    return false;
}

b8 eventFire(u16 code, void* sender, EventContext context) {
    if (isInitialized == false) {
        return false;
    }

    /* If nothing is registered fot the code, boot out. */
    if (state.registered[code].events == 0) {
        return false;
    }

    u64 registeredCount = dynamicArrayLength(state.registered[code].events);
    for (u64 i = 0; i < registeredCount; ++i) {
        RegisteredEvent event = state.registered[code].events[i];

        if (event.callback(code, sender, event.listener, context)) {
            /* Message has been handled, do not send to other listeners. */
            return true;
        }
    }

    /* Not found. */
    return false;
}
