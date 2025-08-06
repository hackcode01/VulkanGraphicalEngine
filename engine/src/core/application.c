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
    if (!initialized) {
        ENGINE_ERROR("ApplicationCreate called more than once.");
        return FALSE;
    }

    appState.gameInstance = gameInstance;

    /* Initialize subsystems. */
    initializeLogging();

    ENGINE_FATAL("A message: %f");
    ENGINE_ERROR("A message: %f",);
    ENGINE_WARNING("A message: %f");
    ENGINE_INFO("A message: %f");
    ENGINE_DEBUG("A message: %f");
    ENGINE_TRACE("A message: %f");

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
}

