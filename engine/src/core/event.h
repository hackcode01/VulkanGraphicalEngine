#ifndef __EVENT_H__
#define __EVENT_H__

#include "../defines.h"

typedef struct EventContext {
    /* 128 bytes. */
    union {
        i64 int64[2];
        u64 uint64[2];
        f64 float64[2];

        i32 int32[4];
        u32 uint32[4];
        f32 float32[4];

        i16 int16[8];
        u16 uint16[8];

        i8 int8[16];
        u8 uint8[16];

        char symbols[16];
    } data;
} EventContext;

/* Should return true if handled. */
typedef b8 (*PFN_on_event)(u16 code, void* sender, void* listenerInstance,
    EventContext data);

b8 eventInitialize();
void eventShutdown();

/**
 * Register to listen for when events are sent with the provided code. Events with duplicate
 * listener/callback combos will not be registered again and will cause this to return false.
 * @param code The event code to listen for.
 * @param listener A pointer to a listener instance. Can be 0/NULL.
 * @param on_event The callback function pointer to be invoked when the event code is fired.
 * @returns true if the event is successfully registered; otherwise false.
 */
ENGINE_API b8 eventRegister(u16 code, void* listener, PFN_on_event on_event);

/**
 * Unregister from listening for when events are sent with the provided code. If no matching
 * registration is found, this function returns false.
 * @param code The event code to stop listening for.
 * @param listener A pointer to a listener instance. Can be 0/NULL.
 * @param on_event The callback function pointer to be unregistered.
 * @returns true if the event is successfully unregistered; otherwise false.
 */
ENGINE_API b8 eventUnregister(u16 code, void* listener, PFN_on_event on_event);

/**
 * Fires an event to listeners of the given code. If an event handler returns 
 * true, the event is considered handled and is not passed on to any more listeners.
 * @param code The event code to fire.
 * @param sender A pointer to the sender. Can be 0/NULL.
 * @param data The event data.
 * @returns true if handled, otherwise false.
 */
ENGINE_API b8 eventFire(u16 code, void* sender, EventContext context);

/* System internal event codes. Application should use codes beyound 255. */
typedef enum SystemEventCode {
    /* Shuts the application down on the next frame. */
    EVENT_CODE_APPLICATION_QUIT = 0x01,

    /**
     * Keyboard key pressed.
     * Context usage:
     * u16 key_code = data.data.uint16[0];
     */
    EVENT_CODE_KEY_PRESSED = 0x02,

    /**
     * Keyboard key released.
     * Context usage:
     * u16 key_code = data.data.u16[0];
     */
    EVENT_CODE_KEY_RELEASED = 0x03,

    /**
     * Mouse button pressed.
     * Context usage:
     * u16 button = data.data.u16[0];
     */
    EVENT_CODE_BUTTON_PRESSED = 0x04,

    /**
     * Mouse button released.
     * Context usage:
     * u16 button = data.data.u16[0];
     */
    EVENT_CODE_BUTTON_RELEASED = 0x05,

    /**
     * Mouse moved.
     * Context usage:
     * u16 x = data.data.u16[0];
     * u16 y = data.data.u16[1];
     */
    EVENT_CODE_MOUSE_MOVED = 0x06,

    /**
     * Mouse moved.
     * Context usage:
     * u8 z_delta = data.data.u8[0];
     */
    EVENT_CODE_MOUSE_WHEEL = 0x07,

    /**
     * Resized/resolution changed from the OS.
     * Context usage:
     * u16 width = data.data.u16[0];
     * u16 height = data.data.u16[1];
     */
    EVENT_CODE_RESIZED = 0x08,

    MAX_EVENT_CODE = 0xFF
} SystemEventCode;

#endif /* __EVENT_H__ */
