#ifndef __ENGINE_ASSERTS_H__
#define __ENGINE_ASSERTS_H__

#include "../defines.h"

/** Disable assertions by commenting out the below ling */
#define ENGINE_ASSERTIONS_ENABLED

#ifdef ENGINE_ASSERTIONS_ENABLED

#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif

ENGINE_API void reportAssertionFailure(const char *expression, const char *message, const char *file, i32 line);

#define ENGINE_ASSERT(expression) {                                  \
    if (expression) {} else {                                        \
        reportAssertionFailure(#expression, "", __FILE__, __LINE__); \
        debugBreak();                                                \
    }                                                                \
}

#define ENGINE_ASSERT_MESSAGE(expression, message) {                      \
    if (expression) {} else {                                             \
        reportAssertionFailure(#expression, message, __FILE__, __LINE__); \
        debugBreak();                                                     \
    }                                                                     \
}

#ifdef _DEBUG
#define ENGINE_ASSERT_DEBUG(expression) {                            \
    if (expression) {} else {                                        \
        reportAssertionFailure(#expression, "", __FILE__, __LINE__); \
        debugBreak();                                                \
    }                                                                \
}

#else
/** Does nothing at all. */
#define ENGINE_ASSERT_DEBUG(expression)
#endif

#else
#define ENGINE_ASSERT(expression)
#define ENGINE_ASSERT_MESSAGE(expression, message)
#define ENGINE_ASSERT_DEBUG(expression)
#endif

#endif
