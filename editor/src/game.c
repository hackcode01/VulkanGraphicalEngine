#include "game.h"

#include "../../engine/src/core/logger.h"

b8 gameInitialize(Game* gameInstance) {
    ENGINE_DEBUG("gameInitialize() called!")

    return TRUE;
}

b8 gameUpdate(Game* gameInstance, f32 deltaTime) {
    return TRUE;
}

b8 gameRender(Game* gameInstance, f32 deltaTime) {
    return TRUE;
}

void gameOnResize(Game* gameInstance, u32 width, u32 height) {}
