#ifndef __ENGINE_STRING_H__
#define __ENGINE_STRING_H__

#include "../defines.h"

/** Returns the length of the given string. */
ENGINE_API u64 stringLength(const char* str);
ENGINE_API char* stringDuplicate(const char* str);

/**
 * Case-sensitive string comparison. True if the same, otherwise false.
 */
ENGINE_API b8 stringsEqual(const char* str_1, const char* str_2);

/** Performs string formatting to dest given format string and parameters. */
ENGINE_API i32 stringFormat(char* dest, const char* format, ...);

/**
 * Performs variadic string formatting to dest given format string and va_list.
 * @param dest The destination for the formatted string.
 * @param format The string to be formatted.
 * @param vaList The variadic argument list.
 * @returns The size of the data written.
 */
ENGINE_API i32 stringFormatV(char* dest, const char* format, void* p_vaList);

#endif /** __ENGINE_STRING_H__ */
