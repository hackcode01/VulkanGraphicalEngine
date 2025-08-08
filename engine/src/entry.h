#ifndef __ENTRY_H__
#define __ENTRY_H__

#include "core/application.h"
#include "core/logger.h"
#include "core/engine_memory.h"
#include "./game_types.h"

#include <stdlib.h>

/* Externally-defined function to create a game. */
extern b8 createGame(Game* outGame);

/**
 * The main entry point of the application.
 */
int main(void) {
    initializeMemory();

    /* Request the game instance from the application. */
    Game gameInstance;
    if (!createGame(&gameInstance)) {
        ENGINE_FATAL("Could not create game!");
        return FAILED_CREATE_GAME;
    }

    /* Ensure the function pointers exist. */
    if (!gameInstance.render || !gameInstance.update || !gameInstance.initialize ||
        !gameInstance.onResize) {
        ENGINE_FATAL("The game's function pointers must be assigned!")
        return FAILED_ASSIGNED_FUNCTION_GAME;
    }

    /* Initialization. */
    if (!applicationCreate(&gameInstance)) {
        ENGINE_INFO("Application failed to create.");
        return FAILED_CREATE_APPLICATION;
    }

    /* Begin the game loop. */
    if (!applicationRun()) {
        ENGINE_INFO("Application didn't shutdown gracefully.")
        return FAILED_APPLICATION_SHUTDOWN_GRACEFULLY;
    }

    shutdownMemory();

    return EXIT_SUCCESS;
}

#endif
