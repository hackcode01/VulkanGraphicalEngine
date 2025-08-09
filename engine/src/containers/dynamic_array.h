#ifndef __DYNAMIC_ARRAY_H__
#define __DYNAMIC_ARRAY_H__

#include "../defines.h"

/**
 * Memory layout
 * u64 capacity = number elements that can be held
 * u64 length = number of elements currently contained
 * u64 stride = size of each element in bytes
 * void* elements
 */
enum {
    DYNAMIC_ARRAY_CAPACITY,
    DYNAMIC_ARRAY_LENGTH,
    DYNAMIC_ARRAY_STRIDE,
    DYNAMIC_ARRAY_FIELD_LENGTH
};

ENGINE_API void* _dynamicArrayCreate(u64 length, u64 stride);
ENGINE_API void _dynamicArrayDestroy(void* array);

ENGINE_API u64 _dynamicArrayFieldGet(void* array, u64 field);
ENGINE_API void _dynamicArrayFieldSet(void* array, u64 field, u64 value);

ENGINE_API void* _dynamicArrayResize(void* array);

ENGINE_API void* _dynamicArrayPush(void* array, const void* valuePtr);
ENGINE_API void _dynamicArrayPop(void* array, void* dest);

ENGINE_API void* _dynamicArrayPopAt(void* array, u64 index, void* dest);
ENGINE_API void* _dynamicArrayInsertAt(void* array, u64 index, void* valuePtr);

#define DYNAMIC_ARRAY_DEFAULT_CAPACITY 1
#define DYNAMIC_ARRAY_RESIZE_FACTOR 2

#define dynamicArrayCreate(type) \
    _dynamicArrayCreate(DYNAMIC_ARRAY_DEFAULT_CAPACITY, sizeof(type))

#define dynamicArrayReserve(type, capacity) \
    _dynamicArrayCreate(capacity, sizeof(type))

#define dynamicArrayDestroy(array) \
    _dynamicArrayDestroy(array)

#define dynamicArrayPush(array, value) {     \
    typeof(value) temp = value;              \
    array = _dynamicArrayPush(array, &temp); \
}

/**
 * NOTE: could use __auto_type for temp above, but intellisense
 * for VSCode flags it as an unknown type. typeof() seems to
 * work just fine, though. Both are GNU extensions.
 */

#define dynamicArrayPop(array, valuePtr) \
    _dynamicArrayPop(array, valuePtr)

#define dynamicInsertAt(array, index, value) {          \
    typeof(value) temp = value;                         \
    array = _dynamicArrayInsertAt(array, index, &temp); \
}

#define dynamicArrayPopAt(array, index, valuePtr) \
    _dynamicArrayPopAt(array, index, valuePtr)

#define dynamicArrayClear(array) \
    _dynamicArrayFieldSet(array, DYNAMIC_ARRAY_LENGTH, 0)

#define dynamicArrayCapacity(array) \
    _dynamicArrayFieldGet(array, DYNAMIC_ARRAY_CAPACITY)

#define dynamicArrayLength(array) \
    _dynamicArrayFieldGet(array, DYNAMIC_ARRAY_LENGTH)

#define dynamicArrayStride(array) \
    _dynamicArrayFieldGet(array, DYNAMIC_ARRAY_STRIDE)

#define dynamicArrayLengthSet(array, value) \
    _dynamicArrayFieldSet(array, DYNAMIC_ARRAY_LENGTH, value)

#endif /* __DYNAMIC_ARRAY_H__ */
