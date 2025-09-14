#ifndef __ENGINE_HASHTABLE_H__
#define __ENGINE_HASHTABLE_H__

#include "../defines.h"

/**
 * @brief Represents a simple hashtable. Members of this structure
 * should not be modified outside the functions associated with it.
 * 
 * For non-pointer types, table retains a copy of the value.For 
 * pointer types, make sure to use the _ptr setter and getter. Table
 * does not take ownership of pointers or associated memory allocations,
 * and should be managed externally.
 */
typedef struct Hashtable {
    u64 elementSize;
    u32 elementCount;
    b8 isPointerType;
    void *memory;
} Hashtable;

/**
 * @brief Creates a hashtable and stores it in out_hashtable.
 * 
 * @param elementSize The size of each element in bytes.
 * @param elementCount The maximum number of elements. Cannot be resized.
 * @param memory A block of memory to be used. Should be equal in size to element_size * element_count;
 * @param isPointerType Indicates if this hashtable will hold pointer types.
 * @param outHashtable A pointer to a hashtable in which to hold relevant data.
 */
ENGINE_API void hashtableCreate(u64 elementSize, u32 elementCount, void *memory,
    b8 isPointerType, Hashtable *outHashtable);

/**
 * @brief Destroys the provided hashtable. Does not release memory for pointer types.
 * 
 * @param table A pointer to the table to be destroyed.
 */
ENGINE_API void hashtableDestroy(Hashtable *table);

/**
 * @brief Stores a copy of the data in value in the provided hashtable. 
 * Only use for tables which were *NOT* created with is_pointer_type = true.
 * 
 * @param table A pointer to the table to get from. Required.
 * @param name The name of the entry to set. Required.
 * @param value The value to be set. Required.
 * @return True, or false if a null pointer is passed.
 */
ENGINE_API b8 hashtableSet(Hashtable *table, const char *name, void *value);

/**
 * @brief Stores a pointer as provided in value in the hashtable.
 * Only use for tables which were created with is_pointer_type = true.
 * 
 * @param table A pointer to the table to get from. Required.
 * @param name The name of the entry to set. Required.
 * @param value A pointer value to be set. Can pass 0 to 'unset' an entry.
 * @return True; or false if a null pointer is passed or if the entry is 0.
 */
ENGINE_API b8 hashtableSetPtr(Hashtable *table, const char *name, void **value);

/**
 * @brief Obtains a copy of data present in the hashtable.
 * Only use for tables which were *NOT* created with is_pointer_type = true.
 * 
 * @param table A pointer to the table to retrieved from. Required.
 * @param name The name of the entry to retrieved. Required.
 * @param value A pointer to store the retrieved value. Required.
 * @return True; or false if a null pointer is passed.
 */
ENGINE_API b8 hashtableGet(Hashtable *table, const char *name, void *outValue);

/**
 * @brief Obtains a pointer to data present in the hashtable.
 * Only use for tables which were created with is_pointer_type = true.
 * 
 * @param table A pointer to the table to retrieved from. Required.
 * @param name The name of the entry to retrieved. Required.
 * @param value A pointer to store the retrieved value. Required.
 * @return True if retrieved successfully; false if a null pointer is passed or is the retrieved value is 0.
 */
ENGINE_API b8 hashtableGetPtr(Hashtable *table, const char *name, void **outValue);

/**
 * @brief Fills all entries in the hashtable with the given value.
 * Useful when non-existent names should return some default value.
 * Should not be used with pointer table types.
 * 
 * @param table A pointer to the table filled. Required.
 * @param value The value to be filled with. Required.
 * @return True if successful; otherwise false.
 */
ENGINE_API b8 hashtableFill(Hashtable *table, void *value);

#endif
