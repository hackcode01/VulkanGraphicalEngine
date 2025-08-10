#include "application.h"
#include "../game_types.h"

#include "logger.h"

#include "../platform/platform.h"
#include "engine_memory.h"
#include "event.h"
#include "input.h"
#include "clock.h"

#include "../renderer/renderer_frontend.h"

#include "../../../editor/src/game.h"

#include <stdio.h>

typedef struct {
    Game* gameInstance;
    b8 isRunning;
    b8 isSuspended;
    PlatformState platform;
    i16 width;
    i16 height;
    clock clock;
    f64 lastTime;
} ApplicationState;

static b8 initialized = FALSE;
static ApplicationState appState;

/* Event handlers. */
b8 applicationOnEvent(u16 code, void* sender, void* listenerInstance, EventContext context);
b8 applicationOnKey(u16 code, void* sender, void* listenerInstance, EventContext context);

b8 applicationCreate(Game* gameInstance) {
    if (initialized) {
        ENGINE_ERROR("ApplicationCreate called more than once.")
        return FALSE;
    }

    appState.gameInstance = gameInstance;

    /* Initialize subsystems. */
    initializeLogging();
    inputInitialize();

    appState.isRunning = TRUE;
    appState.isSuspended = FALSE;

    if (!eventInitialize()) {
        ENGINE_ERROR("Event system failed initialization. Application cannot continue.")
        return FALSE;
    }

    eventRegister(EVENT_CODE_APPLICATION_QUIT, 0, applicationOnEvent);
    eventRegister(EVENT_CODE_KEY_PRESSED, 0, applicationOnKey);
    eventRegister(EVENT_CODE_KEY_RELEASED, 0, applicationOnKey);

    if (!platformStartup(
        &appState.platform,
        gameInstance->appConfig.name,
        gameInstance->appConfig.startPositionX,
        gameInstance->appConfig.startPositionY,
        gameInstance->appConfig.startWidth,
        gameInstance->appConfig.startHeight)) {
        return FALSE;
    }

    /** Renderer startup */
    if (!rendererInitialize(gameInstance->appConfig.name, &appState.platform)) {
        ENGINE_FATAL("Failed to initialize renderer. Aborting application.")
        return FALSE;
    }

    /* Initialize the game. */
    if (!appState.gameInstance->initialize(appState.gameInstance)) {
        ENGINE_FATAL("Game failed to initialize")
        return FALSE;
    }

    appState.gameInstance->onResize(appState.gameInstance, appState.width, appState.height);
    initialized = TRUE;

    return TRUE;
}

b8 applicationRun() {
    clockStart(&appState.clock);
    clockUpdate(&appState.clock);
    appState.lastTime = appState.clock.elapsed;
    f64 runningTime = 0;
    u8 frameCount = 0;
    f64 targetFrameSeconds = 1.0f / 60.0f;

    printf("\n\n");
    ENGINE_DEBUG("Size of struct GameState = %lld", sizeof(GameState))
    ENGINE_DEBUG("Size of struct Game = %lld", sizeof(Game))
    ENGINE_DEBUG("Size of struct Game = %lld", sizeof(ApplicationState))
    ENGINE_DEBUG("Size of struct Game = %lld", sizeof(ApplicationConfig_t))
    ENGINE_DEBUG("Size of struct PlatformState = %lld", sizeof(PlatformState))
    printf("\n\n");

    ENGINE_INFO(engineGetMemoryUsageStr())

    while (appState.isRunning) {
        if (!platformPumpMessages(&appState.platform)) {
            appState.isRunning = FALSE;
        }

        if (!appState.isSuspended) {
            /** Update clock and get delta time. */
            clockUpdate(&appState.clock);
            f64 currentTime = appState.clock.elapsed;
            f64 delta = (currentTime - appState.lastTime);
            f64 frameStartTime = platformGetAbsoluteTime();

            if (!appState.gameInstance->update(appState.gameInstance, (f32)delta)) {
                ENGINE_FATAL("Game update failed, shutting down.")
                appState.isRunning = FALSE;
                break;
            }

            /* Call the game's render routine. */
            if (!appState.gameInstance->render(appState.gameInstance, (f32)delta)) {
                ENGINE_FATAL("Game render failed, shutting down.")
                appState.isRunning = FALSE;
                break;
            }

            /**
             * Refactor packet creation.
             */
            RenderPacket packet;
            packet.deltaTime = delta;
            rendererDrawFrame(&packet);

            /** Figure out how long the frame took and, if below. */
            f64 frameEndTime = platformGetAbsoluteTime();
            f64 frameElapsedTime = frameEndTime - frameStartTime;
            runningTime += frameElapsedTime;
            f64 remainingSeconds = targetFrameSeconds - frameElapsedTime;

            if (remainingSeconds > 0) {
                u64 remaining_ms = (remainingSeconds * 1000);

                /** If there is time left, give it back to the OS. */
                b8 limitFrames = FALSE;
                if (remaining_ms > 0 && limitFrames) {
                    platformSleep(remaining_ms -1);
                }

                ++frameCount;
            }

            /**
             * NOTE: Input update/state copying should always be handled
             * after any input should be recorded. I.E. before this line.
             * As a safety, input is the last thing to be updated before
             * this frame ends.
             */
            inputUpdate(delta);

            /** Update last time. */
            appState.lastTime = currentTime;
        }
    }

    appState.isRunning = FALSE;

    /* Shutdown event system. */
    eventUnregister(EVENT_CODE_APPLICATION_QUIT, 0, applicationOnEvent);
    eventUnregister(EVENT_CODE_KEY_PRESSED, 0, applicationOnKey);
    eventUnregister(EVENT_CODE_KEY_RELEASED, 0, applicationOnKey);
    eventShutdown();
    inputShutdown();

    rendererShutdown();

    platformShutdown(&appState.platform);

    return TRUE;
}

b8 applicationOnEvent(u16 code, void* sender, void* listenerInstance,
    EventContext context) {
    switch (code) {
        case EVENT_CODE_APPLICATION_QUIT: {
            printf("\n\n");
            ENGINE_INFO("EVENT_CODE_APPLICATION_QUIT recieved, shutting down.\n")
            appState.isRunning = FALSE;
            return TRUE;
        }
    }

    return FALSE;
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
            return TRUE;
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

    return FALSE;
}
