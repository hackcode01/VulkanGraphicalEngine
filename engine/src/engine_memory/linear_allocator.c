#include "linear_allocator.h"

#include "../engine_memory/engine_memory.h"
#include "../core/logger.h"

void linearAllocatorCreate(u64 totalSize, void* memory, LinearAllocator* outAllocator) {
    if (outAllocator) {
        outAllocator->totalSize = totalSize;
        outAllocator->allocated = 0;
        outAllocator->ownsMemory = memory == 0;
        if (memory) {
            outAllocator->memory = memory;
        } else {
            outAllocator->memory = engineAllocate(totalSize, MEMORY_TAG_LINEAR_ALLOCATOR);
        }
    }
}
void linearAllocatorDestroy(LinearAllocator* allocator) {
    if (allocator) {
        allocator->allocated = 0;
        if (allocator->ownsMemory && allocator->memory) {
            engineFree(allocator->memory, allocator->totalSize, MEMORY_TAG_LINEAR_ALLOCATOR);
        } 
        allocator->memory = 0;
        allocator->totalSize = 0;
        allocator->ownsMemory = false;
    }
}

void* linearAllocatorAllocate(LinearAllocator* allocator, u64 size) {
    if (allocator && allocator->memory) {
        if (allocator->allocated + size > allocator->totalSize) {
            u64 remaining = allocator->totalSize - allocator->allocated;
            ENGINE_ERROR("linear_allocator_allocate - Tried to allocate %lluB, only %lluB remaining.", size, remaining);
            return 0;
        }

        void* block = ((u8*)allocator->memory) + allocator->allocated;
        allocator->allocated += size;

        return block;
    }

    ENGINE_ERROR("linear_allocator_allocate - provided allocator not initialized.");

    return 0;
}

void linearAllocatorFreeAll(LinearAllocator* allocator) {
    if (allocator && allocator->memory) {
        allocator->allocated = 0;
        engineZeroMemory(allocator->memory, allocator->totalSize);
    }
}
