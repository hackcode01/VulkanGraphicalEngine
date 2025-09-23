#ifndef __ENGINE_STRING_H__
#define __ENGINE_STRING_H__

#include "../defines.h"
#include "../engine_math/math_types.h"

/** Returns the length of the given string. */
ENGINE_API u64 stringLength(const char* str);
ENGINE_API char* stringDuplicate(const char* str);

/**
 * Case-sensitive string comparison. True if the same, otherwise false.
 */
ENGINE_API b8 stringsEqual(const char *str_1, const char *str_2);

/** Case-insensitive string comparison. True if the same, otherwise false. */
ENGINE_API b8 stringsEquali(const char *str_1, const char *str_2);

/** Performs string formatting to dest given format string and parameters. */
ENGINE_API i32 stringFormat(char *dest, const char *format, ...);

/**
 * Performs variadic string formatting to dest given format string and va_list.
 * @param dest The destination for the formatted string.
 * @param format The string to be formatted.
 * @param vaList The variadic argument list.
 * @returns The size of the data written.
 */
ENGINE_API i32 stringFormatV(char *dest, const char *format, void *p_vaList);

ENGINE_API char *stringCopy(char *dest, const char *source);

ENGINE_API char *stringNCopy(char *dest, const char *source, i64 length);

ENGINE_API char *stringTrim(char *str);

ENGINE_API void stringMid(char *dest, const char *source, i32 start, i32 length);

/**
 * @brief Returns the index of the first occurance of c in str; otherwise -1.
 * 
 * @param str The string to be scanned.
 * @param symbol The character to search for.
 * @return The index of the first occurance of c; otherwise -1 if not found. 
 */
ENGINE_API i32 stringIndexOf(char *str, char symbol);

/**
 * @brief Attempts to parse a vector from the provided string.
 * 
 * @param str The string to parse from. Should be space-delimited. (i.e. "1.0 2.0 3.0 4.0")
 * @param outVector A pointer to the vector to write to.
 * @return True if parsed successfully; otherwise false.
 */
ENGINE_API b8 stringToVec4(char *str, vec4 *outVector);

/**
 * @brief Attempts to parse a vector from the provided string.
 * 
 * @param str The string to parse from. Should be space-delimited. (i.e. "1.0 2.0 3.0")
 * @param outVector A pointer to the vector to write to.
 * @return True if parsed successfully; otherwise false.
 */
ENGINE_API b8 stringToVec3(char *str, vec3 *outVector);

/**
 * @brief Attempts to parse a vector from the provided string.
 * 
 * @param str The string to parse from. Should be space-delimited. (i.e. "1.0 2.0")
 * @param outVector A pointer to the vector to write to.
 * @return True if parsed successfully; otherwise false.
 */
ENGINE_API b8 stringToVec2(char *str, vec2 *outVector);

/**
 * @brief Attempts to parse a 32-bit floating-point number from the provided string.
 * 
 * @param str The string to parse from. Should *not* be postfixed with 'f'.
 * @param number A pointer to the float to write to.
 * @return True if parsed successfully; otherwise false.
 */
ENGINE_API b8 stringToF32(char *str, f32 *number);

/**
 * @brief Attempts to parse a 64-bit floating-point number from the provided string.
 * 
 * @param str The string to parse from.
 * @param number A pointer to the float to write to.
 * @return True if parsed successfully; otherwise false.
 */
ENGINE_API b8 stringToF64(char *str, f64 *number);

/**
 * @brief Attempts to parse an 8-bit signed integer from the provided string.
 * 
 * @param str The string to parse from.
 * @param number A pointer to the int to write to.
 * @return True if parsed successfully; otherwise false.
 */
ENGINE_API b8 stringToI8(char *str, i8 *number);

/**
 * @brief Attempts to parse a 16-bit signed integer from the provided string.
 * 
 * @param str The string to parse from.
 * @param number A pointer to the int to write to.
 * @return True if parsed successfully; otherwise false.
 */
ENGINE_API b8 stringToI16(char *str, i16 *number);

/**
 * @brief Attempts to parse a 32-bit signed integer from the provided string.
 * 
 * @param str The string to parse from.
 * @param number A pointer to the int to write to.
 * @return True if parsed successfully; otherwise false.
 */
ENGINE_API b8 stringToI32(char *str, i32 *number);

/**
 * @brief Attempts to parse a 64-bit signed integer from the provided string.
 * 
 * @param str The string to parse from.
 * @param number A pointer to the int to write to.
 * @return True if parsed successfully; otherwise false.
 */
ENGINE_API b8 stringToI64(char *str, i64 *number);

/**
 * @brief Attempts to parse an 8-bit unsigned integer from the provided string.
 * 
 * @param str The string to parse from.
 * @param number A pointer to the int to write to.
 * @return True if parsed successfully; otherwise false.
 */
ENGINE_API b8 stringToU8(char *str, u8 *number);

/**
 * @brief Attempts to parse a 16-bit unsigned integer from the provided string.
 * 
 * @param str The string to parse from.
 * @param number A pointer to the int to write to.
 * @return True if parsed successfully; otherwise false.
 */
ENGINE_API b8 stringToU16(char *str, u16 *number);

/**
 * @brief Attempts to parse a 32-bit unsigned integer from the provided string.
 * 
 * @param str The string to parse from.
 * @param number A pointer to the int to write to.
 * @return True if parsed successfully; otherwise false.
 */
ENGINE_API b8 stringToU32(char *str, u32 *number);

/**
 * @brief Attempts to parse a 64-bit unsigned integer from the provided string.
 * 
 * @param str The string to parse from.
 * @param number A pointer to the int to write to.
 * @return True if parsed successfully; otherwise false.
 */
ENGINE_API b8 stringToU64(char *str, u64 *number);

/**
 * @brief Attempts to parse a boolean from the provided string.
 * "true" or "1" are considered true; anything else is false.
 * 
 * @param str The string to parse from. "true" or "1" are considered true; anything else is false.
 * @param boolValue A pointer to the boolean to write to.
 * @return True if parsed successfully; otherwise false.
 */
ENGINE_API b8 stringToBool(char *str, b8 *boolValue);

#endif
