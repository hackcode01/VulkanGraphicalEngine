#include "event.h"

#include "engine_memory.h"
#include "../src/containers/dynamic_array.h"

typedef struct RegisteredEvent {
    void* listener;
    PFN_on_event callback;
} RegisteredEvent;

typedef struct EventCodeEntry {
    RegisteredEvent* events;
} EventCodeEntry;

#define MAX_MESSAGES_CODES 65536

/* State structure. */
typedef struct EventSystemState {
    /* Lookup table for event codes. */
    EventCodeEntry registered[MAX_MESSAGES_CODES];
} EventSystemState;

/**
 * Event system internal state.
 */
static b8 isInitialized = FALSE;
static EventSystemState state;

b8 eventInitialize() {
    if (isInitialized == TRUE) {
        return FALSE;
    }

    isInitialized = FALSE;
    engineZeroMemory(&state, sizeof(state));

    isInitialized = TRUE;

    return TRUE;
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
    if (isInitialized == FALSE) {
        return FALSE;
    }

    if (state.registered[code].events == 0) {
        state.registered[code].events == dynamicArrayCreate(RegisteredEvent);
    }

    u64 registeredCount = dynamicArrayLength(state.registered[code].events);
    for (u64 i = 0; i < registeredCount; ++i) {
        if (state.registered[code].events[i].listener == listener) {
            return FALSE;
        }
    }

    /* If at this point, no duplicate was found. Proceed with registration. */
    RegisteredEvent event;
    event.listener = listener;
    event.callback = on_event;
    dynamicArrayPush(state.registered[code].events, event);

    return TRUE;
}

b8 eventUnregister(u16 code, void* listener, PFN_on_event on_event) {
    if (isInitialized == FALSE) {
        return FALSE;
    }

    /* On nothing is registered for the code, boot out. */
    if (state.registered[code].events == 0) {
        return FALSE;
    }

    u64 registeredCount = dynamicArrayLength(state.registered[code].events);
    for (u64 i = 0; i < registeredCount; ++i) {
        RegisteredEvent event = state.registered[code].events[i];

        if (event.listener == listener && event.callback == on_event) {
            /* Found one, remove it. */
            RegisteredEvent popped_event;
            dynamicArrayPopAt(state.registered[code].events, i, &popped_event);
            return TRUE;
        }
    }

    /* Not found. */
    return FALSE;
}

b8 eventFire(u16 code, void* sender, EventContext context) {
    if (isInitialized == FALSE) {
        return FALSE;
    }

    /* If nothing is registered fot the code, boot out. */
    if (state.registered[code].events == 0) {
        return FALSE;
    }

    u64 registeredCount = dynamicArrayLength(state.registered[code].events);
    for (u64 i = 0; i < registeredCount; ++i) {
        RegisteredEvent event = state.registered[code].events[i];

        if (event.callback(code, sender, event.listener, context)) {
            /* Message has been handled, do not send to other listeners. */
            return TRUE;
        }
    }

    /* Not found. */
    return FALSE;
}
