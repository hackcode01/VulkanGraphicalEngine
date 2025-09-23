#include "engine_string.h"
#include "engine_memory.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

#ifndef _MSC_VER
#include <string.h>
#endif

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

b8 stringsEquali(const char *str_1, const char *str_2) {
#if defined(__GNUC__)
    return strcasecmp(str_1, str_2) == 0;
#elif (defined _MSC_VER)
    return _strcmpi(str_1, str_2) == 0;
#endif
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

char *stringCopy(char *dest, const char *source) {
    return strcpy(dest, source);
}

char *stringNCopy(char *dest, const char *source, i64 length) {
    return strncpy(dest, source, length);
}

char *stringTrim(char *str) {
    while (isspace((unsigned char)*str)) {
        str++;
    }

    if (*str) {
        char *ptr = str;
        while (*ptr) {
            ptr++;
        }

        while (isspace((unsigned char)*(--ptr))) {
            ;
        }

        ptr[1] = '\0';
    }

    return str;
}

void stringMid(char *dest, const char *source, i32 start, i32 length) {
    if (length == 0) {
        return;
    }

    u64 srcLength = stringLength(source);
    if (start >= srcLength) {
        dest[0] = 0;
        return;
    }

    if (length > 0) {
        for (u64 i = start, j = 0; j < length && source[i]; ++i, ++j) {
            dest[j] = source[i];
        }
        dest[start + length] = 0;
    } else {
        u64 j = 0;
        for (u64 i = start; source[i]; ++i, ++j) {
            dest[j] = source[i];
        }
        dest[start + j] = 0;
    }
}

i32 stringIndexOf(char *str, char symbol) {
    if (!str) {
        return -1;
    }

    u32 length = stringLength(str);
    if (length > 0) {
        for (u32 i = 0; i < length; ++i) {
            if (str[i] == symbol) {
                return i;
            }
        }
    }

    return -1;
}

b8 stringToVec4(char *str, vec4 *outVector) {
    if (!str) {
        return false;
    }

    engineZeroMemory(outVector, sizeof(vec4));
    i32 result = sscanf(str, "%f %f %f %f", &outVector->x, &outVector->y,
        &outVector->z, &outVector->w);

    return result != -1;
}

b8 stringToVec3(char *str, vec3 *outVector) {
    if (!str) {
        return false;
    }

    engineZeroMemory(outVector, sizeof(vec3));
    i32 result = sscanf(str, "%f %f %f", &outVector->x, &outVector->y, &outVector->z);

    return result != -1;
}

b8 stringToVec2(char *str, vec2 *outVector) {
    if (!str) {
        return false;
    }

    engineZeroMemory(outVector, sizeof(vec2));
    i32 result = sscanf(str, "%f %f", &outVector->x, &outVector->y);

    return result != -1;
}

b8 stringToF32(char *str, f32 *number) {
    if (!str) {
        return false;
    }

    *number = 0;
    i32 result = sscanf(str, "%f", number);

    return result != -1;
}

b8 stringToF64(char *str, f64 *number) {
    if (!str) {
        return false;
    }

    *number = 0;
    i32 result = sscanf(str, "%lf", number);

    return result != -1;
}

b8 stringToI8(char *str, i8 *number) {
    if (!str) {
        return false;
    }

    *number = 0;
    i32 result = sscanf(str, "%hhi", number);

    return result != -1;
}

b8 stringToI16(char *str, i16 *number) {
    if (!str) {
        return false;
    }

    *number = 0;
    i32 result = sscanf(str, "%hi", number);

    return result != -1;
}

b8 stringToI32(char *str, i32 *number) {
    if (!str) {
        return false;
    }

    *number = 0;
    i32 result = sscanf(str, "%i", number);

    return result != -1;
}

b8 stringToI64(char *str, i64 *number) {
    if (!str) {
        return false;
    }

    *number = 0;
    i32 result = sscanf(str, "%lli", number);

    return result != -1;
}

b8 stringToU8(char *str, u8 *number) {
    if (!str) {
        return false;
    }

    *number = 0;
    i32 result = sscanf(str, "%hhu", number);

    return result != -1;
}

b8 stringToU16(char *str, u16 *number) {
    if (!str) {
        return false;
    }

    *number = 0;
    i32 result = sscanf(str, "%hu", number);

    return result != -1;
}

b8 stringToU32(char *str, u32 *number) {
    if (!str) {
        return false;
    }

    *number = 0;
    i32 result = sscanf(str, "%u", number);

    return result != -1;
}

b8 stringToU64(char *str, u64 *number) {
    if (!str) {
        return false;
    }

    *number = 0;
    i32 result = sscanf(str, "%llu", number);

    return result != -1;
}

b8 stringToBool(char *str, b8 *boolValue) {
    if (!str) {
        return false;
    }

    return stringsEqual(str, "1") || stringsEquali(str, "true");
}
