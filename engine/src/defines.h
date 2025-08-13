#ifndef __DEFINES_H__
#define __DEFINES_H__

/* Unsigned int types. */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Signed int types. */
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

/* Floating point types. */
typedef float f32;
typedef double f64;

/* Boolean types. */
typedef int b32;
typedef char b8;

/* Properly define static assertions. */
#if defined(__clang__) || defined(__gcc__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

/* Ensure all types of the correct size. */
#if defined(__clang__) && defined(_WIN32)
STATIC_ASSERT(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u8 to be 2 byte.");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u8 to be 4 byte.");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u8 to be 8 byte.");

STATIC_ASSERT(sizeof(i8) == 1, "Expected u8 to be 1 byte.");
STATIC_ASSERT(sizeof(i16) == 2, "Expected u8 to be 1 byte.");
STATIC_ASSERT(sizeof(i32) == 4, "Expected u8 to be 1 byte.");
STATIC_ASSERT(sizeof(i64) == 8, "Expected u8 to be 1 byte.");

STATIC_ASSERT(sizeof(f32) == 4, "Expected u8 to be 1 byte.");
STATIC_ASSERT(sizeof(f64) == 8, "Expected u8 to be 1 byte.");
#endif

#define TRUE 1
#define FALSE 0

/* Platform detection. */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define PLATFORM_WINDOWS 1
#ifndef _WIN64
#error "64-bit is required on Windows!"
#endif

/* Linux OS. */
#elif defined(__linux__) || defined(__gnu_linux__)
#define PLATFORM_LINUX 1
#if defined(__ANDROID__)
#define PLATFORM_ANDROID 1
#endif

/* Catch anything not caught by the above. */
#define PLATFORM_UNIX 1

/* Posix. */
#elif defined(_POSIX_VERSION)
#define PLATFORM_POSIX 1

/* Apple platforms. */
#define PLATFORM_APPLE 1
#include <TargetConditionals.h>

/* iOS Simulator. */
#if TARGET_IPHONE_SIMULATOR
#define PLATFORM_IOS 1
#define PLATFORM_IOS_SIMULATOR 1
#elif TARGET_OS_IPHONE

/* iOS device. */
#define PLATFORM_IOS 1

/* Other kinds of Mac OS */
#elif TARGET_OS_MAC
#else
#error "Unknown Apple platform."
#endif

#else
#error "Unknown platform!"
#endif

#ifdef ENGINE_EXPORT

/* Exports. */
#ifdef _MSC_VER
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __attribute__((visibility("default")))
#endif

#else
/* Imports. */
#ifdef __MSC_VER
#define ENGINE_API __declspec(dllimport)
#else
#define ENGINE_API
#endif
#endif

#define ENGINE_CLAMP(value, min, max) (value <= min) ? min : (value >= max) ? max : value;

#endif /* __DEFINES_H__ */
