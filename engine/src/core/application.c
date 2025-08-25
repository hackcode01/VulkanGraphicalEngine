#include "application.h"
#include "../game_types.h"

#include "logger.h"

#include "../platform/platform.h"
#include "../engine_memory/engine_memory.h"
#include "event.h"
#include "input.h"
#include "clock.h"
#include "../engine_memory/linear_allocator.h"
#include "../../../editor/src/game.h"

#include "../renderer/renderer_frontend.h"

#include <stdio.h>

typedef struct {
    Game* gameInstance;
    b8 isRunning;
    b8 isSuspended;

    i16 width;
    i16 height;
    Clock clock;
    f64 lastTime;
    LinearAllocator systemsAllocator;

    u64 eventSystemMemoryRequirement;
    void *eventSystemState;

    u64 memorySystemMemoryRequirement;
    void* memorySystemState;

    u64 loggingSystemMemoryRequirement;
    void* loggingSystemState;

    u64 inputSystemMemoryRequirement;
    void *inputSystemState;

    u64 platformSystemMemoryRequirement;
    void *platformSystemState;

    u64 rendererSystemMemoryRequirement;
    void *rendererSystemState;

} ApplicationState;

static ApplicationState* appState;

/* Event handlers. */
b8 applicationOnEvent(u16 code, void* sender, void* listenerInstance, EventContext context);
b8 applicationOnKey(u16 code, void* sender, void* listenerInstance, EventContext context);
b8 applicationOnResize(u16 code, void* sender, void* listenerInstance, EventContext context);

b8 applicationCreate(Game* gameInstance) {
    if (gameInstance->applicationState) {
        ENGINE_ERROR("ApplicationCreate called more than once.")
        return false;
    }

    gameInstance->applicationState = engineAllocate(sizeof(ApplicationState),
        MEMORY_TAG_APPLICATION);
    appState = gameInstance->applicationState;
    appState->gameInstance = gameInstance;
    appState->isRunning = false;
    appState->isSuspended = false;

    u64 systemsAllocatorTotalSize = 64 * 1024 * 1024;
    linearAllocatorCreate(systemsAllocatorTotalSize, 0, &appState->systemsAllocator);

    /** Initialize subsystems. */

    /** Events. */
    eventSystemInitialize(&appState->eventSystemMemoryRequirement, 0);
    appState->eventSystemState = linearAllocatorAllocate(
        &appState->systemsAllocator,
        appState->eventSystemMemoryRequirement
    );
    eventSystemInitialize(
        &appState->eventSystemMemoryRequirement,
        appState->eventSystemState
    );

    /** Memory. */
    memorySystemInitialize(&appState->memorySystemMemoryRequirement, 0);
    appState->memorySystemState = linearAllocatorAllocate(
        &appState->systemsAllocator,
        appState->memorySystemMemoryRequirement
    );
    memorySystemInitialize(
        &appState->memorySystemMemoryRequirement,
        appState->memorySystemState
    );

    /** Logging. */
    initializeLogging(&appState->loggingSystemMemoryRequirement, 0);
    appState->loggingSystemState = linearAllocatorAllocate(&appState->systemsAllocator,
        appState->loggingSystemMemoryRequirement);

    if (!initializeLogging(&appState->loggingSystemMemoryRequirement, appState->loggingSystemState)) {
        ENGINE_ERROR("Failed to initialize logging system; shutting down.");
        return false;
    }

    /** Input. */
    inputSystemInitialize(&appState->inputSystemMemoryRequirement, 0);
    appState->inputSystemState = linearAllocatorAllocate(
        &appState->systemsAllocator,
        appState->inputSystemMemoryRequirement
    );
    inputSystemInitialize(
        &appState->inputSystemMemoryRequirement,
        appState->inputSystemState
    );

    /** Register for engine-level events. */
    eventRegister(EVENT_CODE_APPLICATION_QUIT, 0, applicationOnEvent);
    eventRegister(EVENT_CODE_KEY_PRESSED, 0, applicationOnKey);
    eventRegister(EVENT_CODE_KEY_RELEASED, 0, applicationOnKey);
    eventRegister(EVENT_CODE_RESIZED, 0, applicationOnResize);

    /** Platform. */
    platformSystemStartup(&appState->platformSystemMemoryRequirement, 0, 0, 0, 0, 0, 0);
    appState->platformSystemState = linearAllocatorAllocate(
        &appState->systemsAllocator,
        appState->platformSystemMemoryRequirement
    );

    if (!platformSystemStartup(&appState->platformSystemMemoryRequirement,
        appState->platformSystemState, gameInstance->appConfig.name,
        gameInstance->appConfig.startPositionX, gameInstance->appConfig.startPositionY,
        gameInstance->appConfig.startWidth, gameInstance->appConfig.startHeight)) {
        return false;
    }

    /** Renderer system. */
    rendererSystemInitialize(&appState->rendererSystemMemoryRequirement, 0, 0);
    appState->rendererSystemState = linearAllocatorAllocate(&appState->systemsAllocator,
        appState->rendererSystemMemoryRequirement);

    if (!rendererSystemInitialize(&appState->rendererSystemMemoryRequirement,
        appState->rendererSystemState, gameInstance->appConfig.name)) {
        ENGINE_FATAL("Failed to initialize renderer. Aborting application.");

        return false;
    }

    /* Initialize the game. */
    if (!appState->gameInstance->initialize(appState->gameInstance)) {
        ENGINE_FATAL("Game failed to initialize")
        return false;
    }

    /** Call resize once to ensure the proper size has been set. */
    appState->gameInstance->onResize(appState->gameInstance, appState->width, appState->height);

    return true;
}

b8 applicationRun() {
    appState->isRunning = true;
    clockStart(&appState->clock);
    clockUpdate(&appState->clock);
    appState->lastTime = appState->clock.elapsed;

    f64 target_frame_seconds = 1.0f / 60;

    ENGINE_INFO(engineGetMemoryUsageStr());

    while (appState->isRunning) {
        if (!platformPumpMessages()) {
            appState->isRunning = false;
        }

        if (!appState->isSuspended) {
            // Update clock and get delta time.
            clockUpdate(&appState->clock);
            f64 current_time = appState->clock.elapsed;
            f64 delta = (current_time - appState->lastTime);
            f64 frame_start_time = platformGetAbsoluteTime();

            if (!appState->gameInstance->update(appState->gameInstance, (f32)delta)) {
                ENGINE_FATAL("Game update failed, shutting down.");
                appState->isRunning = false;
                break;
            }

            // Call the game's render routine.
            if (!appState->gameInstance->render(appState->gameInstance, (f32)delta)) {
                ENGINE_FATAL("Game render failed, shutting down.");
                appState->isRunning = false;
                break;
            }

            // TODO: refactor packet creation
            RenderPacket packet;
            packet.deltaTime = delta;
            rendererDrawFrame(&packet);

            // Figure out how long the frame took and, if below
            f64 frame_end_time = platformGetAbsoluteTime();
            f64 frame_elapsed_time = frame_end_time - frame_start_time;
            f64 remaining_seconds = target_frame_seconds - frame_elapsed_time;

            if (remaining_seconds > 0) {
                u64 remaining_ms = (remaining_seconds * 1000);

                // If there is time left, give it back to the OS.
                b8 limit_frames = false;
                if (remaining_ms > 0 && limit_frames) {
                    platformSleep(remaining_ms - 1);
                }
            }

            // NOTE: Input update/state copying should always be handled
            // after any input should be recorded; I.E. before this line.
            // As a safety, input is the last thing to be updated before
            // this frame ends.
            inputUpdate(delta);

            // Update last time
            appState->lastTime = current_time;
        }
    }

    appState->isRunning = false;

    // Shutdown event system.
    eventUnregister(EVENT_CODE_APPLICATION_QUIT, 0, applicationOnEvent);
    eventUnregister(EVENT_CODE_KEY_PRESSED, 0, applicationOnKey);
    eventUnregister(EVENT_CODE_KEY_RELEASED, 0, applicationOnKey);

    inputSystemShutdown(appState->inputSystemState);
    rendererSystemShutdown(appState->rendererSystemState);
    platformSystemShutdown(appState->platformSystemState);

    memorySystemShutdown(appState->memorySystemState);
    eventSystemShutdown(appState->eventSystemState);

    return true;
}

void applicationGetFramebufferSize(u32* width, u32* height) {
    *width = appState->width;
    *height = appState->height;
}

b8 applicationOnEvent(u16 code, void* sender, void* listenerInstance,
    EventContext context) {
    switch (code) {
        case EVENT_CODE_APPLICATION_QUIT: {
            printf("\n\n");
            ENGINE_INFO("EVENT_CODE_APPLICATION_QUIT recieved, shutting down.\n")
            appState->isRunning = false;
            return true;
        }
    }

    return false;
}

b8 applicationOnKey(u16 code, void* sender, void* listenerInstance,
    EventContext context) {
    if (code == EVENT_CODE_KEY_PRESSED) {
        u16 keyCode = context.data.uint16[0];

        if (keyCode == KEY_ESCAPE) {
            /**
             * NOTE: Technically firing an event to itself,
             * but there may be other listeners.
             */
            EventContext data = {};
            eventFire(EVENT_CODE_APPLICATION_QUIT, 0, data);

            /* Block anything else from processing this. */
            return true;
        } else if (keyCode == KEY_A) {
            ENGINE_DEBUG("Explicit - A key pressed!")
        } else {
            ENGINE_DEBUG("'%c' key pressed in window.", keyCode)
        }
    } else if (code == EVENT_CODE_KEY_RELEASED) {
        u16 keyCode = context.data.uint16[0];

        if (keyCode == KEY_B) {
            ENGINE_DEBUG("Explicit - B key released!")
        } else {
            ENGINE_DEBUG("'%c' key released in window", keyCode)
        }
    }

    return false;
}

b8 applicationOnResize(u16 code, void* sender, void* listenerInstance, EventContext context) {
    if (code == EVENT_CODE_RESIZED) {
        u16 width = context.data.uint16[0];
        u16 height = context.data.uint16[1];

        /** Check if different. If so, trigger a resize event. */
        if (width != appState->width || height != appState->height) {
            appState->width = width;
            appState->height = height;

            ENGINE_DEBUG("Window resize: %i, %i", width, height);

            /** Handle minimization. */
            if (width == 0 || height == 0) {
                ENGINE_INFO("Window minimized, saspending application.")
                appState->isSuspended = true;
                return true;
            } else {
                if (appState->isSuspended) {
                    ENGINE_INFO("Window restored, resuming application.")
                    appState->isSuspended = false;
                }
                appState->gameInstance->onResize(appState->gameInstance, width, height);
                rendererOnResized(width, height);
            }
        }
    }

    /** Event purposely not handled to allow other listeners to get this. */
    return false;
}
