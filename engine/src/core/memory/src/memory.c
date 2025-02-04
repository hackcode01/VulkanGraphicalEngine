#include "../includes/memory.h"

#include "../../logger.h"
#include "../../../platforms/includes/platform.h"

/* Custom string lib. */
#include <string.h>
#include <stdio.h>

struct memoryStats {
    u64 totalAllocated;
    u64 taggedAllocations[MEMORY_TAG_MAX_TAGS];
};

static const char* memoryTagStrings[MEMORY_TAG_MAX_TAGS] = {
    "UNKNOWN       ",
    "ARRAY         ",
    "DYNAMIC_ARRAY ",
    "DICTIONARY    ",
    "RING_QUEUE    ",
    "BST           ",
    "STRING        ",
    "APPLICATION   ",
    "JOB           ",
    "TEXTURE       ",
    "MAT_INSTANCE  ",
    "RENDERER      ",
    "GAME          ",
    "TRANSFORM     ",
    "ENTITY        ",
    "ENTITY_NODE   ",
    "SCENE         "
};

static struct memoryStats stats;

void initializeMemory() {
    platformZeroMemory(&stats, sizeof(stats));
}

void shutdownMemory() {}

void* allocate(u64 size, memoryTag_t tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        WARNING("Allocate called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    stats.totalAllocated += size;
    stats.taggedAllocations[tag] += size;

    /* Memory alignment. */
    void* block = platformAllocate(size, FALSE);
    platformZeroMemory(block, size);

    return block;
}

void memoryFree(void* block, u64 size, memoryTag_t tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        WARNING("Free called using MEMORY_TAG_UNKNOWN. Re-class this allocation.");
    }

    stats.totalAllocated -= size;
    stats.taggedAllocations[tag] -= size;

    /* Memory alignment. */
    platformFree(block, FALSE);
}

void* zeroMemory(void* block, u64 size) {
    return platformZeroMemory(block, size);
}

void* copyMemory(void* destination, const void* source, u64 size) {
    return platformCopyMemory(destination, source, size);
}

void* setMemory(void* destination, i32 value, u64 size) {
    return platformSetMemory(destination, value, size);
}

char* getMemoryUsageStr() {
    const u64 gigabyte = 1024 * 1024 * 1024;
    const u64 megabyte = 1024 * 1024;
    const u64 kilobyte = 1024;

    char buffer[8000] = "System memory use (tagged):\n";
    u64 offset = strlen(buffer);

    for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; ++i) {
        char unit[4] = "XiB";
        float amount = 1.0f;

        if (stats.taggedAllocations[i] >= gigabyte) {
            unit[0] = 'G';
            amount = stats.taggedAllocations[i] / (float)gigabyte;
        } else if (stats.taggedAllocations[i] >= megabyte) {
            unit[0] = 'M';
            amount = stats.taggedAllocations[i] / (float)megabyte;
        } else if (stats.taggedAllocations[i] >= kilobyte) {
            unit[0] = 'K';
            amount = stats.taggedAllocations[i] / (float)kilobyte;
        } else {
            unit[0] = 'B';
            unit[1] = 0;
            amount = (float)stats.taggedAllocations[i];
        }

        i32 length = snprintf(buffer + offset, 8000, "%s: %.2f%s\n",
                              memoryTagStrings[i], amount, unit);
        offset += length;
    }

    char* outString = _strdup(buffer);

    return outString;
}
