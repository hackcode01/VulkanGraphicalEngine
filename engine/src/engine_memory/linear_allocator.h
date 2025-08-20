#ifndef __ENGINE_LINEAR_ALLOCATOR_H__
#define __ENGINE_LINEAR_ALLOCATOR_H__

#include "../defines.h"

typedef struct LinearAllocator {
    u64 totalSize;
    u64 allocated;
    void* memory;
    b8 ownsMemory;
} LinearAllocator;

ENGINE_API void linearAllocatorCreate(u64 totalSize, void* memory, LinearAllocator* outAllocator);
ENGINE_API void linearAllocatorDestroy(LinearAllocator* allocator);

ENGINE_API void* linearAllocatorAllocate(LinearAllocator* allocator, u64 size);
ENGINE_API void linearAllocatorFreeAll(LinearAllocator* allocator);

#endif /** __ENGINE_LINEAR_ALLOCATOR_H__ */
