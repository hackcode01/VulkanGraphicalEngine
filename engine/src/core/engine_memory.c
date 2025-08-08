#include "engine_memory.h"

#include "core/logger.h"
#include "platform/platform.h"

#include <string.h>
#include <stdio.h>

struct MemoryStats {
    u64 totalAllocated;
    u64 taggedAllocations[MEMORY_TAG_MAX_TAGS];
};

static const char* memoryTagStrings[MEMORY_TAG_MAX_TAGS] = {
    "UNKNOWN",
    "ARRAY",
    "DARRAY",
    "DICTIONARY",
    "RING_QUEUE",
    "BST",
    "STRING",
    "APPLICATION",
    "JOB",
    "TEXTURE",
    "MATERIAL_INSTANCE",
    "RENDERER",
    "GAME",
    "TRANSFORM",
    "ENTITY",
    "ENTITY_NODE",
    "SCENE"
};

static struct MemoryStats memoryStats;

void initializeMemory() {
    platformZeroMemory(&memoryStats, sizeof(memoryStats));
}

void shutdownMemory() {}

void* engineAllocate(u64 size, MemoryTag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        ENGINE_WARNING("engineAllocate called using MEMORY_TAG_UKNOWN. "
            "Re-class this allocation.")
    }

    memoryStats.totalAllocated += size;
    memoryStats.taggedAllocations[tag] += size;

    void* block = platformAllocate(size, FALSE);
    platformZeroMemory(block, size);

    return block;
}

void engineFree(void* block, u64 size, MemoryTag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        ENGINE_WARNING("engineFree called using MEMORY_TAG_UNKNOW. "
            "Re-class this allocation.");
    }

    memoryStats.totalAllocated -= size;
    memoryStats.taggedAllocations[tag] -= size;

    platformFree(block);
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

        if (memoryStats.taggedAllocations[i] >= gib) {
            unit[0] = 'G';
            amount = memoryStats.taggedAllocations[i] / (float)gib;
        } else if (memoryStats.taggedAllocations[i] >= mib) {
            unit[0] = 'M';
            amount = memoryStats.taggedAllocations[i] / (float)mib;
        } else if (memoryStats.taggedAllocations[i] >= kib) {
            unit[0] = 'K';
            amount = memoryStats.taggedAllocations[i] / (float)kib;
        } else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)memoryStats.taggedAllocations[i];
        }

        i32 length = snprintf(buffer + offset, 8000, "  %s: %.2f%s\n",
            memoryTagStrings[i], amount, unit);
        offset += length;
    }

    char* out_string = strdup(buffer);

    return out_string;
}
