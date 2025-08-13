#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "../defines.h"

struct Game;

typedef struct {
    /* Window starting position x axis, if applicable. */
    i16 startPositionX;

    /* Window starting position y axis, if applicable. */
    i16 startPositionY;

    /* Window starting width, if applicable. */
    i16 startWidth;

    /* Window starting height, if applicable. */
    i16 startHeight;

    /* The application name used in windowing, if applicable. */
    char* name;
} ApplicationConfig_t;

ENGINE_API b8 applicationCreate(struct Game* gameInstance);

ENGINE_API b8 applicationRun();

void applicationGetFramebufferSize(u32* width, u32* height);

#endif
