#ifndef __GAME_H__
#define __GAME_H__

#include "../../engine/src/defines.h"
#include "../../engine/src/game_types.h"
#include "../../engine/src/engine_math/math_types.h"

typedef struct GameState {
    f32 deltaTime;
    mat4 view;
    vec3 cameraPosition;
    vec3 cameraEuler;
    b8 cameraViewDirty;
} GameState;

b8 gameInitialize(Game* gameInstance);

b8 gameUpdate(Game* gameInstance, f32 deltaTime);

b8 gameRender(Game* gameInstance, f32 deltaTime);

void gameOnResize(Game* gameInstance, u32 width, u32 height);

#endif
