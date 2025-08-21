#include "engine_string.h"
#include "engine_memory.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

u64 stringLength(const char* str) {
    return strlen(str);
}

char* stringDuplicate(const char* str) {
    u64 length = stringLength(str);
    char* copy = engineAllocate(length + 1, MEMORY_TAG_STRING);
    engineCopyMemory(copy, str, length + 1);

    return copy;
}

b8 stringsEqual(const char* str_1, const char* str_2) {
    return strcmp(str_1, str_2) == 0;
}

i32 stringFormat(char* dest, const char* format, ...) {
    if (dest) {
        va_list argPtr;
        va_start(argPtr, format);
        i32 written = stringFormatV(dest, format, argPtr);
        va_end(argPtr);

        return written;
    }

    return -1;
}

i32 stringFormatV(char* dest, const char* format, void* p_vaList) {
    if (dest) {
        /** Big, but can fit on the stack. */
        char buffer[32000];
        i32 written = vsnprintf(buffer, 32000, format, p_vaList);
        buffer[written] = 0;
        engineCopyMemory(dest, buffer, written + 1);

        return written;
    }

    return -1;
}
