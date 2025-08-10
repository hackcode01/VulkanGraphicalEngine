#ifndef __ENGINE_STRING_H__
#define __ENGINE_STRING_H__

#include "../defines.h"

/** Returns the length of the given string. */
ENGINE_API u64 stringLength(const char* str);
ENGINE_API char* stringDuplicate(const char* str);

#endif /* __ENGINE_STRING_H__ */
