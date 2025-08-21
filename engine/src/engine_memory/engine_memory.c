#include "engine_memory.h"
#include "engine_string.h"

#include "../core/logger.h"
#include "../platform/platform.h"

#include <string.h>
#include <stdio.h>

struct MemoryStats {
    u64 totalAllocated;
    u64 taggedAllocations[MEMORY_TAG_MAX_TAGS];
};

static const char* memoryTagStrings[MEMORY_TAG_MAX_TAGS] = {
    "UNKNOWN    ",
    "ARRAY      ",
    "LINEAR_ALLC",
    "DARRAY     ",
    "DICT       ",
    "RING_QUEUE ",
    "BST        ",
    "STRING     ",
    "APPLICATION",
    "JOB        ",
    "TEXTURE    ",
    "MAT_INST   ",
    "RENDERER   ",
    "GAME       ",
    "TRANSFORM  ",
    "ENTITY     ",
    "ENTITY_NODE",
    "SCENE      "
};

typedef struct MemorySystemState {
    struct MemoryStats stats;
    u64 allocationCount;
} MemorySystemState;

static MemorySystemState* s_statePtr;

void initializeMemory(u64* memoryRequirement, void* state) {
    *memoryRequirement = sizeof(MemorySystemState);
    if (state == 0) {
        return;
    }

    s_statePtr = state;
    s_statePtr->allocationCount = 0;
    platformZeroMemory(&s_statePtr->stats, sizeof(s_statePtr->stats));
}

void shutdownMemory() {
    s_statePtr = 0;
}

void* engineAllocate(u64 size, MemoryTag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        ENGINE_WARNING("kallocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    if (s_statePtr) {
        s_statePtr->stats.totalAllocated += size;
        s_statePtr->stats.taggedAllocations[tag] += size;
        ++(s_statePtr->allocationCount);
    }

    // TODO: Memory alignment
    void* block = platformAllocate(size, false);
    platformZeroMemory(block, size);
    return block;
}

void engineFree(void* block, u64 size, MemoryTag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        ENGINE_WARNING("kfree called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    s_statePtr->stats.totalAllocated -= size;
    s_statePtr->stats.taggedAllocations[tag] -= size;

    platformFree(block, false);
}

void* engineZeroMemory(void* block, u64 size) {
    return platformZeroMemory(block, size);
}

void* engineCopyMemory(void* dest, const void* source, u64 size) {
    return platformCopyMemory(dest, source, size);
}

void* engineSetMemory(void* dest, i32 value, u64 size) {
    return platformSetMemory(dest, value, size);
}

char* engineGetMemoryUsageStr() {
    const u64 gib = 1024 * 1024 * 1024;
    const u64 mib = 1024 * 1024;
    const u64 kib = 1024;

    char buffer[8000] = "System memory use (tagged):\n";
    u64 offset = strlen(buffer);
    for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) {
        char unit[4] = "XiB";
        float amount = 1.0f;
        if (s_statePtr->stats.taggedAllocations[i] >= gib) {
            unit[0] = 'G';
            amount = s_statePtr->stats.taggedAllocations[i] / (float)gib;
        } else if (s_statePtr->stats.taggedAllocations[i] >= mib) {
            unit[0] = 'M';
            amount = s_statePtr->stats.taggedAllocations[i] / (float)mib;
        } else if (s_statePtr->stats.taggedAllocations[i] >= kib) {
            unit[0] = 'K';
            amount = s_statePtr->stats.taggedAllocations[i] / (float)kib;
        } else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)s_statePtr->stats.taggedAllocations[i];
        }

        i32 length = snprintf(buffer + offset, 8000, "  %s: %.2f%s\n", memoryTagStrings[i], amount, unit);
        offset += length;
    }
    char* out_string = stringDuplicate(buffer);
    return out_string;
}

u64 getMemoryAllocationCount() {
    if (s_statePtr) {
        return s_statePtr->allocationCount;
    }

    return 0;
}
