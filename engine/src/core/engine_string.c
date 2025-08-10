#include "engine_string.h"
#include "engine_memory.h"

#include <string.h>

u64 stringLength(const char* str) {
    return strlen(str);
}

char* stringDuplicate(const char* str) {
    u64 length = stringLength(str);
    char* copy = engineAllocate(length + 1, MEMORY_TAG_STRING);
    engineCopyMemory(copy, str, length + 1);

    return copy;
}
