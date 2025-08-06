#ifndef __GAME_TYPES_H__
#define __GAME_TYPES_H__

#include "core/application.h"

typedef struct Game {
    /* The application configuration. */
    ApplicationConfig_t appConfig;

    /* Function pointer to game's initialize function. */
    b8 (*initialize)(struct Game* gameInstance);

    /* Function pointer to game's update function. */
    b8 (*update)(struct Game* gameInstance, f32 deltaTime);

    /* Function pointer to game's render function. */
    b8 (*render)(struct Game* gameInstance, f32 deltaTime);

    /* Function pointer to handle resizes, if applicable. */
    void (*onResize)(struct Game* gameInstance, u32 width, u32 height);

    /* Game-specific game state. Created and managed by the game. */
    void* state;
} Game;

#endif
