#include "game.h"

#include "../../engine/src/core/logger.h"

b8 gameInitialize(Game* gameInstance) {
    ENGINE_DEBUG("gameInitialize() called!")

    return true;
}

b8 gameUpdate(Game* gameInstance, f32 deltaTime) {
    return true;
}

b8 gameRender(Game* gameInstance, f32 deltaTime) {
    return true;
}

void gameOnResize(Game* gameInstance, u32 width, u32 height) {}
