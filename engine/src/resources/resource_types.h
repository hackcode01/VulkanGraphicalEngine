#ifndef __ENGINE_RESOURCE_TYPES_H__
#define __ENGINE_RESOURCE_TYPES_H__

#include "../engine_math/math_types.h"

typedef struct Texture {
    u32 id;
    u32 width;
    u32 height;

    u8 channelCount;
    b8 hasTransparency;
    u32 generation;
    void *internalData;
} Texture;

#endif
