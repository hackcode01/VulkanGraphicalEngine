#ifndef ASSERTS_H
#define ASSERTS_H

#include "../defines.h"

/* Disable assertions by commenting out the below line. */
#define ASSERTIONS_ENABLED

#ifdef ASSERTIONS_ENABLED

#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()

#else
#define debugBreak() __builtin_trap()
#endif

API void reportAssertionFailure(const char* expression, const char* message,
                                const char* file, i32 lineNumber);

#define ASSERT(expression)                                               \
    {                                                                    \
        if (expression) {                                                \
        } else {                                                         \
            reportAssertionFailure(#expression, "", __FILE__, __LINE__); \
            debugBreak();                                                \
        }                                                                \
    }

#define ASSERT_MSG(expression, message)                                       \
    {                                                                         \
        if (expression) {                                                     \
        } else {                                                              \
            reportAssertionFailure(#expression, message, __FILE__, __LINE__); \
            debugBreak();                                                     \
        }                                                                     \
    }

#ifdef _DEBUG
#define ASSERT_DEBUG(expression)                                         \
    {                                                                    \
        if (expression) {                                                \
        } else {                                                         \
            reportAssertionFailure(#expression, "", __FILE__, __LINE__); \
            debugBreak();                                                \
        }                                                                \
    }

#else
/* Does nothing at all. */
#define ASSERT_DEBUG(expression)
#endif

#else
/* Does nothing at all. */
#define ASSERT(expression)

/* Does nothing at all. */
#define ASSERT_MSG(expression, message)

/* Does nothing at all. */
#define ASSERT_DEBUG(expression)
#endif

#endif
