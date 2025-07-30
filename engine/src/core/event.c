#include "./event.h"

#include "./memory/includes/memory.h"
#include "../containers/includes/dynamic_array.h"

typedef struct registeredEvent_t {
    void* listener;
    PFNOnEvent callback;
} registeredEvent_t;

typedef struct eventCodeEntry_t {
    registeredEvent_t* events;
} eventCodeEntry_t;

/* This should be more than enough codes... */
#define MAX_MESSAGE_CODES 16384

/* State structure. */
typedef struct eventSystemState_t {
    /* Lookup table for event codes. */
    eventCodeEntry_t registered[MAX_MESSAGE_CODES];
} eventSystemState_t;

/**
  * Event system internal state.
  */
static b8 isInitialized = FALSE;
static eventSystemState_t state;

b8 eventInitialize() {
    if (isInitialized == TRUE) {
        return FALSE;
    }

    isInitialized = FALSE;
    zeroMemory(&state, sizeof(state));

    isInitialized = TRUE;

    return TRUE;
}

void eventShutdown() {
    /* Free the events arrays. And objects pointed to should be destroyed on their own. */
    for (u16 i = 0; i < MAX_MESSAGE_CODES; ++i){
        if (state.registered[i].events != 0) {
            dynamicArrayDestroy(state.registered[i].events);
            state.registered[i].events = 0;
        }
    }
}

b8 eventRegister(u16 code, void* listener, PFNOnEvent onEvent) {
    if (isInitialized == FALSE) {
        return FALSE;
    }

    if (state.registered[code].events == 0) {
        state.registered[code].events = dynamicArrayCreate(registeredEvent_t);
    }

    u64 registeredCount = dynamicArrayLength(state.registered[code].events);
    for (u64 i = 0; i < registeredCount; ++i) {
        if (state.registered[code].events[i].listener == listener) {
            return FALSE;
        }
    }

    /* If at this point, no duplicate was found. Proceed with registration. */
    registeredEvent_t event;
    event.listener = listener;
    event.callback = onEvent;
    dynamicArrayPush(state.registered[code].events, event);

    return TRUE;
}

b8 eventUnregister(u16 code, void* listener, PFNOnEvent onEvent) {
    if (isInitialized == FALSE) {
        return FALSE;
    }

    /* On nothing is registered for the code, boot out. */
    if (state.registered[code].events == 0) {
        return FALSE;
    }

    u64 registeredCount = dynamicArrayLength(state.registered[code].events);
    for (u64 i = 0; i < registeredCount; ++i) {
        registeredEvent_t event = state.registered[code].events[i];
        if (event.listener == listener && event.callback == onEvent) {
            /* Found one, remove it. */
            registeredEvent_t poppedEvent;
            dynamicArrayPopAt(state.registered[code].events, i, &poppedEvent);
            return TRUE;
        }
    }

    /* Not found. */
    return FALSE;
}

b8 eventFire(u16 code, void* sender, eventContext_t context) {
    if (isInitialized == FALSE) {
        return FALSE;
    }

    /* If nothing is registered for the code, boot out. */
    if (state.registered[code].events == 0) {
        return FALSE;
    }

    u64 registeredCount = dynamicArrayLength(state.registered[code].events);
    for (u64 i = 0; i < registeredCount; ++i) {
        registeredEvent_t event = state.registered[code].events[i];

        if (event.callback(code, sender, event.listener, context)) {
            /* Message has been handled, do not send to other listeners. */
            return TRUE;
        }
    }

    /* Not found. */
    return FALSE;
}
