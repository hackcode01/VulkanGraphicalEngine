#include "../includes/dynamic_array.h"

#include "../../core/memory/includes/memory.h"
#include "../../core/logger.h"

void* _dynamicArrayCreate(u64 length, u64 stride) {
    u64 headerSize = DYNAMIC_ARRAY_FIELD_LENGTH * sizeof(u64);
    u64 arraySize = length * stride;
    u64* newArray = allocate(headerSize + arraySize, MEMORY_TAG_DYNAMIC_ARRAY);

    setMemory(newArray, 0, headerSize + arraySize);

    newArray[DYNAMIC_ARRAY_CAPACITY] = length;
    newArray[DYNAMIC_ARRAY_LENGTH] = 0;
    newArray[DYNAMIC_ARRAY_STRIDE] = stride;

    return (void*)(newArray + DYNAMIC_ARRAY_FIELD_LENGTH);
}

void _dynamicArrayDestroy(void* array) {
    u64* header = (u64*)array - DYNAMIC_ARRAY_FIELD_LENGTH;
    u64 headerSize = DYNAMIC_ARRAY_FIELD_LENGTH * sizeof(u64);
    u64 totalSize = headerSize + header[DYNAMIC_ARRAY_CAPACITY] *
                     header[DYNAMIC_ARRAY_STRIDE];
    memoryFree(header, totalSize, MEMORY_TAG_DYNAMIC_ARRAY);
}

u64 _dynamicArrayFieldGet(void* array, u64 field) {
    u64* header = (u64*)array - DYNAMIC_ARRAY_FIELD_LENGTH;

    return header[field];
}

void _dynamicArrayFieldSet(void* array, u64 field, u64 value) {
    u64* header = (u64*)array - DYNAMIC_ARRAY_FIELD_LENGTH;
    header[field] = value;
}

void* _dynamicArrayResize(void* array) {
    u64 length = dynamicArrayLength(array);
    u64 stride = dynamicArrayStride(array);
    void* temp = _dynamicArrayCreate(
        (DARRAY_RESIZE_FACTOR * dynamicArrayCapacity(array)),
        stride);
    copyMemory(temp, array, length * stride);

    _dynamicArrayFieldSet(temp, DYNAMIC_ARRAY_LENGTH, length);
    _dynamicArrayDestroy(array);

    return temp;
}

void* _dynamicArrayPush(void* array, const void* valuePtr) {
    u64 length = dynamicArrayLength(array);
    u64 stride = dynamicArrayStride(array);

    if (length >= dynamicArrayCapacity(array)) {
        array = _dynamicArrayResize(array);
    }

    u64 address = (u64)array;
    address += (length * stride);
    copyMemory((void*)address, valuePtr, stride);
    _dynamicArrayFieldSet(array, DYNAMIC_ARRAY_LENGTH, length + 1);

    return array;
}

void _dynamicArrayPop(void* array, void* destination) {
    u64 length = dynamicArrayLength(array);
    u64 stride = dynamicArrayStride(array);
    u64 address = (u64)array;

    address += ((length - 1) * stride);
    copyMemory(destination, (void*)address, stride);
    _dynamicArrayFieldSet(array, DYNAMIC_ARRAY_LENGTH, length - 1);
}

void* _dynamicArrayPopAt(void* array, u64 index, void* destination) {
    u64 length = dynamicArrayLength(array);
    u64 stride = dynamicArrayStride(array);

    if (index >= length) {
        ERROR("Index outside the bounds of this array! Length: %i, index: %index", length, index);

        return array;
    }

    u64 address = (u64)array;
    copyMemory(destination, (void*)(address + (index * stride)), stride);

    /* If not on the last element, snip out the entry and copy the rest inward. */
    if (index != length - 1) {
        copyMemory((void*)(address + (index * stride)),
                   (void*)(address + ((index + 1) * stride)),
                   stride * (length - index));
    }

    _dynamicArrayFieldSet(array, DYNAMIC_ARRAY_LENGTH, length - 1);

    return array;
}

void* _dynamicArrayInsertAt(void* array, u64 index, void* valuePtr) {
    u64 length = dynamicArrayLength(array);
    u64 stride = dynamicArrayStride(array);

    if (index >= length) {
        ERROR("Index outside the bounds of this array! Length: %i, index: %index",
              length, index);

        return array;
    }

    if (length >= dynamicArrayCapacity(array)) {
        array = _dynamicArrayResize(array);
    }

    u64 address = (u64)array;

    /* If not on the last element, copy the rest outward. */
    if (index != length - 1) {
        copyMemory((void*)(address + ((index + 1) * stride)),
                   (void*)(address + (index * stride)),
                   stride * (length - index));
    }

    /* Set the value at the index. */
    copyMemory((void*)(address + (index * stride)), valuePtr, stride);

    _dynamicArrayFieldSet(array, DYNAMIC_ARRAY_LENGTH, length + 1);

    return array;
}
