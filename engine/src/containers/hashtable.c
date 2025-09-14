#include "hashtable.h"

#include "../engine_memory/engine_memory.h"

#include "../core/logger.h"

u64 hashName(const char *name, u32 elementCount) {
    /** A simular to use when generating a hash. Prime to hopefully avoid collisions. */
    static const u64 multiplier = 97;

    unsigned const char *us;
    u64 hash = 0;

    for (us = (unsigned const char *)name; *us; us++) {
        hash = hash * multiplier + *us;
    }

    /** Mod it against the size of the table. */
    hash %= elementCount;

    return hash;
}

void hashtableCreate(u64 elementSize, u32 elementCount, void *memory, b8 isPointerType,
    Hashtable *outHashtable) {

    if (!memory || !outHashtable) {
        ENGINE_ERROR("hashtableCreate failed! Pointer to memory and outHashtable are required!")
        return;
    }

    if (!elementCount || !elementSize) {
        ENGINE_ERROR("elementSize and elementCount must be a positive non-zero value!")
        return;
    }

    outHashtable->memory = memory;
    outHashtable->elementCount = elementCount;
    outHashtable->elementSize = elementSize;
    outHashtable->isPointerType = isPointerType;

    engineZeroMemory(outHashtable->memory, elementSize * elementCount);
}

void hashtableDestroy(Hashtable *table) {
    if (table) {
        engineZeroMemory(table, sizeof(Hashtable));
    }
}

b8 hashtableSet(Hashtable *table, const char *name, void *value) {
    if (!table || !name || !value) {
        ENGINE_ERROR("hashtableSet requires tables, name and value to exist.")
        return false;
    }

    if (table->isPointerType) {
        ENGINE_ERROR("hashtableSet should not be used with tables that have pointer types. "
            "Use hashtableSetPtr instead.")
        return false;
    }

    u64 hash = hashName(name, table->elementCount);
    engineCopyMemory(table->memory + (table->elementSize * hash), value,
                     table->elementSize);

    return true;
}

b8 hashtableSetPtr(Hashtable *table, const char *name, void **value) {
    if (!table || !name) {
        ENGINE_WARNING("hashtableSetPtr requires table and name to exist.")
        return false;
    }

    if (!table->isPointerType) {
        ENGINE_ERROR("hashtable_set_ptr should not be used with tables that do "
            "not have pointer types. Use hashtable_set instead.")
        return false;
    }

    u64 hash = hashName(name, table->elementCount);
    ((void**)table->memory)[hash] = value ? *value : 0;

    return true;
}

b8 hashtableGet(Hashtable *table, const char *name, void *outValue) {
    if (!table || !name || !outValue) {
        ENGINE_WARNING("hashtableGet requires table, name and outValue to exist.")
        return false;
    }

    if (table->isPointerType) {
        ENGINE_ERROR("hashtableGet should not be used with tables that have pointer types. "
            "Use hashtableSetPtr instead.")
        return false;
    }

    u64 hash = hashName(name, table->elementCount);
    engineCopyMemory(outValue, table->memory + (table->elementSize * hash),
                     table->elementSize);

    return true;
}

b8 hashtableGetPtr(Hashtable *table, const char *name, void **outValue) {
    if (!table || !name || !outValue) {
        ENGINE_WARNING("hashtableGetPtr requires table, name and outValue to exist.")
        return false;
    }

    if (!table->isPointerType) {
        ENGINE_ERROR("hashtable_get_ptr should not be used with tables that do not have pointer types. "
            "Use hashtable_get instead.")
        return false;
    }

    u64 hash = hashName(name, table->elementCount);
    *outValue = ((void**)table->memory)[hash];

    return *outValue != 0;
}

b8 hashtableFill(Hashtable *table, void *value) {
    if (!table || !value) {
        ENGINE_WARNING("hashtable_fill requires table and value to exist.")
        return false;
    }

    if (table->isPointerType) {
        ENGINE_ERROR("hashtable_fill should not be used with tables that have pointer types.")
        return false;
    }

    for (u32 i = 0; i < table->elementCount; ++i) {
        engineCopyMemory(table->memory + (table->elementSize * i), value, table->elementSize);
    }

    return true;
}
