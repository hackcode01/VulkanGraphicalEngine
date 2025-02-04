#include "./game.h"

#include "../../engine/src/core/logger.h"

b8 gameInitialize(game_t* gameInstance) {
    DEBUG("gameInitialize() called!");

    return TRUE;
}

b8 gameUpdate(game_t* gameInstance, f32 deltaTime) {
    return TRUE;
}

b8 gameRender(game_t* gameInstance, f32 deltaTime) {
    return TRUE;
}

void gameOnResize(game_t* gameInstance, u32 width, u32 height) {}
