#ifndef MEMORY_H
#define MEMORY_H

#include "../../../defines.h"

typedef enum memoryTag {
    /* For temporary use. Should be assigned one of the below or have a new tag created. */
    MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_DYNAMIC_ARRAY,
    MEMORY_TAG_DICTIONARY,
    MEMORY_TAG_RING_QUEUE,
    MEMORY_TAG_BST,
    MEMORY_TAG_STRING,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_JOB,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL_INSTANCE,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_GAME,
    MEMORY_TAG_TRANSFORM,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_ENTITY_NODE,
    MEMORY_TAG_SCENE,

    MEMORY_TAG_MAX_TAGS
} memoryTag_t;

API void initializeMemory();
API void shutdownMemory();

API void* allocate(u64 size, memoryTag_t tag);

API void memoryFree(void* block, u64 size, memoryTag_t tag);

API void* zeroMemory(void* block, u64 size);

API void* copyMemory(void* destination, const void* source, u64 size);

API void* setMemory(void* destination, i32 value, u64 size);

API char* getMemoryUsageStr();

#endif
