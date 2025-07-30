#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "../../defines.h"

typedef struct {
    void* internalState;
} platformState_t;

b8 platformStartup(platformState_t* platformState,
                       const char* applicationName,
                       i32 x, i32 y, i32 width, i32 height);

void platformShutdown(platformState_t* platformState);

b8 platformPumpMessages(platformState_t* platformState);

void* platformAllocate(u64 size, b8 aligned);

void platformFree(void* block, b8 aligned);

void* platformZeroMemory(void* block, u64 size);

void* platformCopyMemory(void* destination, const void* source, u64 size);

void* platformSetMemory(void* destination, i32 value, u64 size);

void platformConsoleWrite(const char* message, u8 colour);

void platformConsoleWriteError(const char* message, u8 colour);

f64 platformGetAbsoluteTime();

/* Sleep on the thread for the provided ms. This blocks the main thread.
 * Should only be used for giving time back to the OS for unused update power.
 * Therefore it is not exported.
 */
void platformSleep(u64 ms);

#endif
