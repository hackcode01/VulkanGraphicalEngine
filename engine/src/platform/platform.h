#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "../defines.h"

b8 platformSystemStartup(
    u64 *memoryRequirement,
    void *state,
    const char *applicationName,
    i32 x,
    i32 y,
    i32 width,
    i32 height
);

void platformSystemShutdown(void *state);

b8 platformPumpMessages();

void* platformAllocate(u64 size, b8 aligned);
void platformFree(void* block, b8 aligned);

void* platformZeroMemory(void* block, u64 size);
void* platformCopyMemory(void* dest, const void* source, u64 size);
void* platformSetMemory(void* dest, i32 value, u64 size);

void platformConsoleWrite(const char* message, u8 color);
void platformConsoleWriteError(const char* message, u8 color);

f64 platformGetAbsoluteTime();

/** Sleep on the thread for the provided ms. This blocks the main thread.
 *  Should only be used for giving time back to the OS for unused update power.
 *  Therefore it is not exported.
 */
void platformSleep(u64 ms);

#endif
