#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include "../../defines.h"

/*
Memory layout
u64 capacity = number elements that can be held
u64 length = number of elements currently contained
u64 stride = size of each element in bytes
void* elements
*/

enum DYNAPIC_ARRAY_PROPERTIES {
    DYNAMIC_ARRAY_CAPACITY,
    DYNAMIC_ARRAY_LENGTH,
    DYNAMIC_ARRAY_STRIDE,
    DYNAMIC_ARRAY_FIELD_LENGTH
};

API void* _dynamicArrayCreate(u64 length, u64 stride);
API void _dynamicArrayDestroy(void* array);

API u64 _dynamicArrayFieldGet(void* array, u64 field);
API void _dynamicArrayFieldSet(void* array, u64 field, u64 value);

API void* _dynamicArrayResize(void* array);

API void* _dynamicArrayPush(void* array, const void* valuePtr);
API void _dynamicArrayPop(void* array, void* destination);

API void* _dynamicArrayPopAt(void* array, u64 index, void* destination);
API void* _dynamicArrayInsertAt(void* array, u64 index, void* valuePtr);

#define DARRAY_DEFAULT_CAPACITY 1
#define DARRAY_RESIZE_FACTOR    2

#define dynamicArrayCreate(type) \
    _dynamicArrayCreate(DARRAY_DEFAULT_CAPACITY, sizeof(type))

#define dynamicArrayReserve(type, capacity) \
    _dynamicArrayCreate(capacity, sizeof(type))

#define dynamicArrayDestroy(array) _dynamicArrayDestroy(array);

#define dynamicArrayPush(array, value)           \
    {                                            \
        typeof(value) temp = value;              \
        array = _dynamicArrayPush(array, &temp); \
    }

#define dynamicArrayPop(array, valuePtr) \
    _dynamicArrayPop(array, valuePtr)

#define dynamicArrayInsertAt(array, index, value)           \
    {                                                       \
        typeof(value) temp = value;                         \
        array = _dynamicArrayInsertAt(array, index, &temp); \
    }

#define dynamicArrayPopAt(array, index, valuePtr) \
    _dynamicArrayPopAt(array, index, valuePtr)

#define dynamicArrayClear(array) \
    _dynamicArrayFieldSet(array, DYNAMIC_ARRAY_FIELD_LENGTH, 0)

#define dynamicArrayCapacity(array) \
    _dynamicArrayFieldGet(array, DYNAMIC_ARRAY_CAPACITY)

#define dynamicArrayLength(array) \
    _dynamicArrayFieldGet(array, DYNAMIC_ARRAY_LENGTH)

#define dynamicArrayStride(array) \
    _dynamicArrayFieldGet(array, DYNAMIC_ARRAY_STRIDE)

#define dynamicArrayLengthSet(array, value) \
    _dynamicArrayFieldSet(array, DYNAMIC_ARRAY_LENGTH, value)


#endif
