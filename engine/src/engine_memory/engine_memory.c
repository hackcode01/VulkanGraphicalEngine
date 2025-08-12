#include "engine_memory.h"
#include "engine_string.h"

#include "../core/logger.h"
#include "../platform/platform.h"

#include <string.h>
#include <stdio.h>

struct MemoryStats {
    u64 total_allocated;
    u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];
};

static const char* memoryTagStrings[MEMORY_TAG_MAX_TAGS] = {
    "UNKNOWN    ",
    "ARRAY      ",
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

static struct MemoryStats stats;

void initializeMemory() {
    platformZeroMemory(&stats, sizeof(stats));
}

void shutdownMemory() {}

void* engineAllocate(u64 size, MemoryTag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        ENGINE_WARNING("kallocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    stats.total_allocated += size;
    stats.tagged_allocations[tag] += size;

    // TODO: Memory alignment
    void* block = platformAllocate(size, FALSE);
    platformZeroMemory(block, size);
    return block;
}

void engineFree(void* block, u64 size, MemoryTag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        ENGINE_WARNING("kfree called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    stats.total_allocated -= size;
    stats.tagged_allocations[tag] -= size;

    platformFree(block, FALSE);
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
        if (stats.tagged_allocations[i] >= gib) {
            unit[0] = 'G';
            amount = stats.tagged_allocations[i] / (float)gib;
        } else if (stats.tagged_allocations[i] >= mib) {
            unit[0] = 'M';
            amount = stats.tagged_allocations[i] / (float)mib;
        } else if (stats.tagged_allocations[i] >= kib) {
            unit[0] = 'K';
            amount = stats.tagged_allocations[i] / (float)kib;
        } else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)stats.tagged_allocations[i];
        }

        i32 length = snprintf(buffer + offset, 8000, "  %s: %.2f%s\n", memoryTagStrings[i], amount, unit);
        offset += length;
    }
    char* out_string = stringDuplicate(buffer);
    return out_string;
}
