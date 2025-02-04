#ifndef GAME_H
#define GAME_H

#include "../../engine/src/defines.h"
#include "../../engine/src/game_types.h"

typedef struct {
    f32 deltaTime;
} gameState_t;

b8 gameInitialize(game_t* gameInstance);

b8 gameUpdate(game_t* gameInstance, f32 deltaTime);

b8 gameRender(game_t* gameInstance, f32 deltaTime);

void gameOnResize(game_t* gameInstance, u32 width, u32 height);

#endif
