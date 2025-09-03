#include "event.h"

#include "../engine_memory/engine_memory.h"
#include "../containers/dynamic_array.h"

typedef struct RegisteredEvent {
    void *listener;
    PFN_on_event callback;
} RegisteredEvent;

typedef struct EventCodeEntry {
    RegisteredEvent *events;
} EventCodeEntry;

#define MAX_MESSAGES_CODES 15000

/** State structure. */
typedef struct EventSystemState {
    /** Lookup table for event codes. */
    EventCodeEntry registered[MAX_MESSAGES_CODES];
} EventSystemState;

/**
 * Event system internal statePtr->
 */
static EventSystemState *statePtr;

void eventSystemInitialize(u64 *memoryRequirement, void *state) {
    *memoryRequirement = sizeof(EventSystemState);
    if (!state) {
        return;
    }

    engineZeroMemory(state, sizeof(state));
    statePtr = state;
}

void eventSystemShutdown(void *state) {
    if (statePtr) {
        /** Free the events arrays. And objects pointed to should be destroyed on their own. */
        for (u16 i = 0; i < MAX_MESSAGES_CODES; ++i) {
            if (statePtr->registered[i].events != 0) {
                dynamicArrayDestroy(statePtr->registered[i].events);
                statePtr->registered[i].events = 0;
            }
        }
    }

    statePtr = 0;
}

b8 eventRegister(u16 code, void *listener, PFN_on_event onEvent) {
    if (!statePtr) {
        return false;
    }

    if (statePtr->registered[code].events == 0) {
        statePtr->registered[code].events = dynamicArrayCreate(RegisteredEvent);
    }

    u64 registeredCount = dynamicArrayLength(statePtr->registered[code].events);
    for (u64 i = 0; i < registeredCount; ++i) {
        if (statePtr->registered[code].events[i].listener == listener) {
            return false;
        }
    }

    /** If at this point, no duplicate was found. Proceed with registration. */
    RegisteredEvent event;
    event.listener = listener;
    event.callback = onEvent;
    dynamicArrayPush(statePtr->registered[code].events, event)

    return true;
}

b8 eventUnregister(u16 code, void *listener, PFN_on_event onEvent) {
    if (!statePtr) {
        return false;
    }

    /** On nothing is registered for the code, boot out. */
    if (statePtr->registered[code].events == 0) {
        return false;
    }

    u64 registeredCount = dynamicArrayLength(statePtr->registered[code].events);
    for (u64 i = 0; i < registeredCount; ++i) {
        RegisteredEvent event = statePtr->registered[code].events[i];

        if (event.listener == listener && event.callback == onEvent) {
            /** Found one, remove it. */
            RegisteredEvent poppedEvent;
            dynamicArrayPopAt(statePtr->registered[code].events, i, &poppedEvent);

            return true;
        }
    }

    /** Not found. */
    return false;
}

b8 eventFire(u16 code, void *sender, EventContext context) {
    if (!statePtr) {
        return false;
    }

    /** If nothing is registered fot the code, boot out. */
    if (statePtr->registered[code].events == 0) {
        return false;
    }

    u64 registeredCount = dynamicArrayLength(statePtr->registered[code].events);
    for (u64 i = 0; i < registeredCount; ++i) {
        RegisteredEvent event = statePtr->registered[code].events[i];

        if (event.callback(code, sender, event.listener, context)) {
            /** Message has been handled, do not send to other listeners. */
            return true;
        }
    }

    /** Not found. */
    return false;
}
