#include "../../engine/src/core/application.h"
#include "../../engine/src/core/logger.h"
#include "../../engine/src/engine_memory/engine_memory.h"
#include "../../engine/src/game_types.h"

#include "game.h"

/** Define the function to create a game. */
b8 createGame(Game* outGame) {
    // Application configuration.
    outGame->appConfig.startPositionX = 100;
    outGame->appConfig.startPositionY = 100;
    outGame->appConfig.startWidth = 1280;
    outGame->appConfig.startHeight = 720;
    outGame->appConfig.name = "Engine Editor";
    outGame->update = gameUpdate;
    outGame->render = gameRender;
    outGame->initialize = gameInitialize;
    outGame->onResize = gameOnResize;

    /** Create the game state. */
    outGame->state = engineAllocate(sizeof(GameState), MEMORY_TAG_GAME);

    return TRUE;
}


int main(void) {
    initializeMemory();

    /* Request the game instance from the application. */
    Game gameInstance;
    if (!createGame(&gameInstance)) {
        ENGINE_FATAL("Could not create game!")
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

    return 0;
}
