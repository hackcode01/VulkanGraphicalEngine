#include "containers/dynamic_array.h"

#include "core/engine_memory.h"
#include "core/logger.h"

void* _dynamicArrayCreate(u64 length, u64 stride) {
    u64 header_size = DYNAMIC_ARRAY_FIELD_LENGTH * sizeof(u64);
    u64 array_size = length * stride;
    u64* new_array = engineAllocate(header_size + array_size, MEMORY_TAG_DYNAMIC_ARRAY);
    engineSetMemory(new_array, 0, header_size + array_size);
    new_array[DYNAMIC_ARRAY_CAPACITY] = length;
    new_array[DYNAMIC_ARRAY_LENGTH] = 0;
    new_array[DYNAMIC_ARRAY_STRIDE] = stride;
    return (void*)(new_array + DYNAMIC_ARRAY_FIELD_LENGTH);
}

void _dynamicArrayDestroy(void* array) {
    u64* header = (u64*)array - DYNAMIC_ARRAY_FIELD_LENGTH;
    u64 header_size = DYNAMIC_ARRAY_FIELD_LENGTH * sizeof(u64);
    u64 total_size = header_size + header[DYNAMIC_ARRAY_CAPACITY] * header[DYNAMIC_ARRAY_STRIDE];
    engineFree(header, total_size, MEMORY_TAG_DYNAMIC_ARRAY);
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
        (DYNAMIC_ARRAY_RESIZE_FACTOR * dynamicArrayCapacity(array)),
        stride);
    engineCopyMemory(temp, array, length * stride);

    _dynamicArrayFieldSet(temp, DYNAMIC_ARRAY_LENGTH, length);
    _dynamicArrayDestroy(array);
    return temp;
}

void* _dynamicArrayPush(void* array, const void* value_ptr) {
    u64 length = dynamicArrayLength(array);
    u64 stride = dynamicArrayStride(array);
    if (length >= dynamicArrayCapacity(array)) {
        array = _dynamicArrayResize(array);
    }

    u64 addr = (u64)array;
    addr += (length * stride);
    engineCopyMemory((void*)addr, value_ptr, stride);
    _dynamicArrayFieldSet(array, DYNAMIC_ARRAY_LENGTH, length + 1);
    return array;
}

void _dynamicArrayPop(void* array, void* dest) {
    u64 length = dynamicArrayLength(array);
    u64 stride = dynamicArrayStride(array);
    u64 addr = (u64)array;
    addr += ((length - 1) * stride);
    engineCopyMemory(dest, (void*)addr, stride);
    _dynamicArrayFieldSet(array, DYNAMIC_ARRAY_LENGTH, length - 1);
}

void* _dynamicArrayPopAt(void* array, u64 index, void* dest) {
    u64 length = dynamicArrayLength(array);
    u64 stride = dynamicArrayStride(array);
    if (index >= length) {
        ENGINE_ERROR("Index outside the bounds of this array! Length: %i, index: %index", length, index);
        return array;
    }

    u64 addr = (u64)array;
    engineCopyMemory(dest, (void*)(addr + (index * stride)), stride);

    // If not on the last element, snip out the entry and copy the rest inward.
    if (index != length - 1) {
        engineCopyMemory(
            (void*)(addr + (index * stride)),
            (void*)(addr + ((index + 1) * stride)),
            stride * (length - index));
    }

    _dynamicArrayFieldSet(array, DYNAMIC_ARRAY_LENGTH, length - 1);
    return array;
}

void* _dynamicArrayInsertAt(void* array, u64 index, void* value_ptr) {
    u64 length = dynamicArrayLength(array);
    u64 stride = dynamicArrayStride(array);
    if (index >= length) {
        ENGINE_ERROR("Index outside the bounds of this array! Length: %i, index: %index", length, index);
        return array;
    }
    if (length >= dynamicArrayCapacity(array)) {
        array = _dynamicArrayResize(array);
    }

    u64 addr = (u64)array;

    // If not on the last element, copy the rest outward.
    if (index != length - 1) {
        engineCopyMemory(
            (void*)(addr + ((index + 1) * stride)),
            (void*)(addr + (index * stride)),
            stride * (length - index));
    }

    // Set the value at the index
    engineCopyMemory((void*)(addr + (index * stride)), value_ptr, stride);

    _dynamicArrayFieldSet(array, DYNAMIC_ARRAY_LENGTH, length + 1);
    return array;
}