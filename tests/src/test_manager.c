#include "../include/test_manager.h"

#include "../../engine/src/containers/dynamic_array.h"
#include "../../engine/src/core/logger.h"
#include "../../engine/src/engine_memory/engine_string.h"
#include "../../engine/src/core/clock.h"

typedef struct TestEntry {
    PFN_test function;
    char* desc;
} TestEntry;

static TestEntry* tests;

void testManagerInit() {
    tests = dynamicArrayCreate(TestEntry);
}

void testManagerRegisterTest(u8 (*PFN_test)(), char* desc) {
    TestEntry testEntry;
    testEntry.function = PFN_test;
    testEntry.desc = desc;
    dynamicArrayPush(tests, testEntry);
}

void testManagerRunTests() {
    u32 passed = 0;
    u32 failed = 0;
    u32 skipped = 0;

    u32 count = dynamicArrayLength(tests);

    Clock totalTime;
    clockStart(&totalTime);

    for (u32 i = 0; i < count; ++i) {
        Clock testTime;
        clockStart(&testTime);
        u8 result = tests[i].function();
        clockUpdate(&testTime);

        if (result == true) {
            ++passed;
        } else if (result == BYPASS) {
            ENGINE_WARNING("[SKIPPED]: %s", tests[i].desc);
            ++skipped;
        } else {
            ENGINE_ERROR("[FAILED]: %s", tests[i].desc);
            ++failed;
        }
    
        char status[20];
        stringFormat(status, failed ? "*** %d FAILED ***" : "SUCCESS", failed);
        clockUpdate(&totalTime);
        ENGINE_INFO("Executed %d of %d (skipped %d) %s (%.6f sec / %.6f sec total", i + 1, count, skipped, status, testTime.elapsed, totalTime.elapsed);
    }

    clockStop(&totalTime);

    ENGINE_INFO("Results: %d passed, %d failed, %d skipped.", passed, failed, skipped);
}
