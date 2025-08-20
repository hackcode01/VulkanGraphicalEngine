#ifndef __ENGINE_MEMORY_H__
#define __ENGINE_MEMORY_H__

#include "../defines.h"

/** For temporary use. Should be assigned one of the below or have a new tag created. */
typedef enum MemoryTag {
    MEMORY_TAG_UNKNOWN,
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_LINEAR_ALLOCATOR,
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
} MemoryTag;

ENGINE_API void initializeMemory();
ENGINE_API void shutdownMemory();

ENGINE_API void* engineAllocate(u64 size, MemoryTag tag);

ENGINE_API void engineFree(void* block, u64 size, MemoryTag tag);

ENGINE_API void* engineZeroMemory(void* block, u64 size);

ENGINE_API void* engineCopyMemory(void* dest, const void* source, u64 size);

ENGINE_API void* engineSetMemory(void* dest, i32 value, u64 size);

ENGINE_API char* engineGetMemoryUsageStr();

ENGINE_API u64 getMemoryAllocCount();

#endif
