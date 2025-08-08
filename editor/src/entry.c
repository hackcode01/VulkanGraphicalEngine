#include "game.h"

#include "../../engine/src/entry.h"

#include "../../engine/src/platform/platform.h"

// Define the function to create a game
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

    // Create the game state.
    outGame->state = platformAllocate(sizeof(GameState), FALSE);

    return TRUE;
}
