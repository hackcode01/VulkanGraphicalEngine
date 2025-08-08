#include "application.h"
#include "../game_types.h"

#include "logger.h"

#include "../platform/platform.h"

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

b8 applicationCreate(Game* gameInstance) {
    if (initialized) {
        ENGINE_ERROR("ApplicationCreate called more than once.");
        return FALSE;
    }

    appState.gameInstance = gameInstance;

    /* Initialize subsystems. */
    initializeLogging();

    ENGINE_FATAL("A message: %f", 3.14f);
    ENGINE_ERROR("A message: %f", 3.14f);
    ENGINE_WARNING("A message: %f", 3.14f);
    ENGINE_INFO("A message: %f", 3.14f);
    ENGINE_DEBUG("A message: %f", 3.14f);
    ENGINE_TRACE("A message: %f", 3.14f);

    appState.isRunning = TRUE;
    appState.isSuspended = FALSE;

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
        ENGINE_FATAL("Game failed to initialize");
        return FALSE;
    }

    appState.gameInstance->onResize(appState.gameInstance, appState.width, appState.height);
    initialized = TRUE;

    return TRUE;
}

b8 applicationRun() {
    while (appState.isRunning) {
        if (!platformPumpMessages(&appState.platform)) {
            appState.isRunning = FALSE;
        }

        if (!appState.isSuspended) {
            if (!appState.gameInstance->update(appState.gameInstance, (f32)0)) {
                ENGINE_FATAL("Game update failed, shutting down.");
                appState.isRunning = FALSE;
                break;
            }

            /* Call the game's render routine. */
            if (!appState.gameInstance->render(appState.gameInstance, (f32)0)) {
                ENGINE_FATAL("Game render failed, shutting down.");
                appState.isRunning = FALSE;
                break;
            }
        }
    }

    appState.isRunning = FALSE;

    platformShutdown(&appState.platform);

    return TRUE;
}

