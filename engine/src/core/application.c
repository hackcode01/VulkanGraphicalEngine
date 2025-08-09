#include "application.h"
#include "../game_types.h"

#include "logger.h"

#include "../platform/platform.h"
#include "engine_memory.h"
#include "event.h"
#include "input.h"

#include "../../../editor/src/game.h"

#include <stdio.h>

typedef struct {
    Game* gameInstance;
    b8 isRunning;
    b8 isSuspended;
    PlatformState_t platform;
    i16 width;
    i16 height;
    f64 lastTime;
} ApplicationState_t;

static b8 initialized = FALSE;
static ApplicationState_t appState;

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
        gameInstance->appConfig.startHeight
    )) {
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
    printf("\n\n");

    ENGINE_DEBUG("Size of struct GameState = %lld", sizeof(GameState))
    ENGINE_DEBUG("Size of struct Game = %lld", sizeof(Game))
    ENGINE_DEBUG("Size of struct Game = %lld", sizeof(ApplicationState_t))
    ENGINE_DEBUG("Size of struct Game = %lld", sizeof(ApplicationConfig_t))
    ENGINE_DEBUG("Size of struct PlatformState_t = %lld", sizeof(PlatformState_t))

    printf("\n\n");

    ENGINE_INFO(engineGetMemoryUsageStr())

    while (appState.isRunning) {
        if (!platformPumpMessages(&appState.platform)) {
            appState.isRunning = FALSE;
        }

        if (!appState.isSuspended) {
            if (!appState.gameInstance->update(appState.gameInstance, (f32)0)) {
                ENGINE_FATAL("Game update failed, shutting down.")
                appState.isRunning = FALSE;
                break;
            }

            /* Call the game's render routine. */
            if (!appState.gameInstance->render(appState.gameInstance, (f32)0)) {
                ENGINE_FATAL("Game render failed, shutting down.")
                appState.isRunning = FALSE;
                break;
            }

            /**
             * NOTE: Input update/state copying should always be handled
             * after any input should be recorded. I.E. before this line.
             * As a safety, input is the last thing to be updated before
             * this frame ends.
             */
            inputUpdate(0);
        }
    }

    appState.isRunning = FALSE;

    /* Shutdown event system. */
    eventUnregister(EVENT_CODE_APPLICATION_QUIT, 0, applicationOnEvent);
    eventUnregister(EVENT_CODE_KEY_PRESSED, 0, applicationOnKey);
    eventUnregister(EVENT_CODE_KEY_RELEASED, 0, applicationOnKey);
    eventShutdown();
    inputShutdown();

    platformShutdown(&appState.platform);

    return TRUE;
}

b8 applicationOnEvent(u16 code, void* sender, void* listenerInstance,
    EventContext context) {
    switch (code) {
        case EVENT_CODE_APPLICATION_QUIT: {
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
