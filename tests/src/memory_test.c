#include "../include/memory_test.h"

#include "../../engine/src/engine_memory/engine_memory.h"

#include <stddef.h>

void logTypeSizes() {
    ENGINE_INFO("Type sizes:\n")

    ENGINE_INFO("signed char: %zu bytes: ", sizeof(i8))
    ENGINE_INFO("signed short: %zu bytes: ", sizeof(i16))
    ENGINE_INFO("signed int: %zu bytes: ", sizeof(i32))
    ENGINE_INFO("signed long long: %zu bytes: ", sizeof(i64))
}

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

void logMemoryUsage() {
    ENGINE_INFO("Memory usage:\n");

#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS processMemoryCounters;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &processMemoryCounters,
        sizeof(PROCESS_MEMORY_COUNTERS))) {

        ENGINE_INFO("WorkingSetSize: %.4f MB",
            (f64)processMemoryCounters.WorkingSetSize / (1024 * 1024))
        ENGINE_INFO("PagefileUsage: %.4f MB",
            (f64)processMemoryCounters.PagefileUsage / (1024 * 1024))
    }
#endif
}
