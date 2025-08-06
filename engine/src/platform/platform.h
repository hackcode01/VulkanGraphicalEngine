#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "../defines.h"

typedef struct {
    void* internalState;
} PlatformState_t;

b8 platformStartup(
    PlatformState_t* platformState,
    const char* applicationName,
    i32 x,
    i32 y,
    i32 width,
    i32 height
);

void platformShutdown(PlatformState_t* platformState);

b8 platformPumpMessages(PlatformState_t* platformState);

ENGINE_API void* platformAllocate(u64 size, b8 aligned);
ENGINE_API void platformFree(void* block);

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
