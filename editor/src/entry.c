#include "./entry.h"

#include "./game.h"

#include "../../engine/src/core/memory/includes/memory.h"

/* Define the function to create a game. */
b8 createGame(game_t* outGame) {
    /* Application configuration. */
    outGame->appConfig.startPositionX = 100;
    outGame->appConfig.startPositionY = 100;
    outGame->appConfig.startWidth = 1280;
    outGame->appConfig.startHeight = 720;
    outGame->appConfig.name = "Graphical Engine Editor";
    outGame->update = gameUpdate;
    outGame->render = gameRender;
    outGame->initialize = gameInitialize;
    outGame->onResize = gameOnResize;

    /* Create the game state. */
    outGame->state = allocate(sizeof(gameState_t), MEMORY_TAG_GAME);

    return TRUE;
}
