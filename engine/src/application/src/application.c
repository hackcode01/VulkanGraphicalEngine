#include "../includes/application.h"
#include "../../game_types.h"

#include "../../core/logger.h"

#include "../../platforms/includes/platform.h"
#include "../../core/memory/includes/memory.h"
#include "../../core/event.h"
#include "../../core/input.h"

typedef struct {
    game_t* gameInstance;
    b8 isRunning;
    b8 isSuspended;
    platformState_t platform;
    i16 width;
    i16 height;
    f64 lastTime;
} applicationState_t;

static b8 initialized = FALSE;
static applicationState_t appState;

/* Event handlers. */
b8 applicationOnEvent(u16 code, void* sender, void* listenerInstance,
                      eventContext_t context);
b8 applicationOnKey(u16 code, void* sender, void* listenerInstance,
                    eventContext_t context);

b8 applicationCreate(game_t* gameInstance) {
    if (initialized) {
        ERROR("application_create called more than once.");

        return FALSE;
    }

    appState.gameInstance = gameInstance;

    /* Initialize subsystems. */
    initializeLogging();
    inputInitialize();

    /* Remove this. */
    FATAL("A test message: %f", 3.14f);
    ERROR("A test message: %f", 3.14f);
    WARNING("A test message: %f", 3.14f);
    INFO("A test message: %f", 3.14f);
    DEBUG("A test message: %f", 3.14f);
    TRACE("A test message: %f", 3.14f);

    appState.isRunning = TRUE;
    appState.isSuspended = FALSE;

    if (!eventInitialize()) {
        ERROR("Event system failed initialization. Application can't continue.")

        return FALSE;
    }

    eventRegister(EVENT_CODE_APPLICATION_QUIT, 0, applicationOnEvent);
    eventRegister(EVENT_CODE_KEY_PRESSED, 0, applicationOnKey);
    eventRegister(EVENT_CODE_KEY_RELEASED, 0, applicationOnKey);

    if (!platformStartup(&appState.platform, gameInstance->appConfig.name,
                         gameInstance->appConfig.startPositionX,
                         gameInstance->appConfig.startPositionY,
                         gameInstance->appConfig.startWidth,
                         gameInstance->appConfig.startHeight)) {
        return FALSE;
    }

    /* Initialize the game. */
    if (!appState.gameInstance->initialize(appState.gameInstance)) {
        FATAL("Game failed to initialize.");
        return FALSE;
    }

    appState.gameInstance->onResize(appState.gameInstance,
                                    appState.width, appState.height);

    initialized = TRUE;

    return TRUE;
}

b8 applicationRun() {
    INFO(getMemoryUsageStr());

    while (appState.isRunning) {
        if (!platformPumpMessages(&appState.platform)) {
            appState.isRunning = FALSE;
        }

        if (!appState.isSuspended) {
            if (!appState.gameInstance->update(appState.gameInstance, (f32)0)) {
                FATAL("Game update failed, shutting down.");
                appState.isRunning = FALSE;
                break;
            }

            /* Call the game's render routine. */
            if (!appState.gameInstance->render(appState.gameInstance, (f32)0)) {
                FATAL("Game render failed, shutting down.");
                appState.isRunning = FALSE;
                break;
            }

            /** Input update/state copying should always be handled
             * after any input should be recorded; I.E. before this line.
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
                      eventContext_t context) {
    switch (code) {
        case EVENT_CODE_APPLICATION_QUIT: {
            INFO("EVENT_CODE_APPLICATION_QUIT recieved, shutting down.\n");
            appState.isRunning = FALSE;
            return TRUE;
        }
    }

    return FALSE;
}

b8 applicationOnKey(u16 code, void* sender, void* listenerInstance, eventContext_t context) {
    if (code == EVENT_CODE_KEY_PRESSED) {
        u16 key_code = context.data_t.u16[0];

        if (key_code == KEY_ESCAPE) {
            /* Technically firing an event to itself, but there may be other listeners. */
            eventContext_t data = {};
            eventFire(EVENT_CODE_APPLICATION_QUIT, 0, data);

            /* Block anything else from processing this. */
            return TRUE;
        } else if (key_code == KEY_A) {
            /* Example on checking for a key. */
            DEBUG("Explicit - A key pressed!");
        } else {
            DEBUG("'%c' key pressed in window.", key_code);
        }
    } else if (code == EVENT_CODE_KEY_RELEASED) {
        u16 key_code = context.data_t.u16[0];

        if (key_code == KEY_B) {
            /* Example on checking for a key. */
            DEBUG("Explicit - B key released!");
        } else {
            DEBUG("'%c' key released in window.", key_code);
        }
    }

    return FALSE;
}
