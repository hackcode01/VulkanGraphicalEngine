#ifndef __ENGINE_TEST_MANAGER_H__
#define __ENGINE_TEST_MANAGER_H__

#include "../../engine/src/defines.h"

#define BYPASS 2

typedef u8 (*PFN_test)();

void testManagerInit();

void testManagerRegisterTest(PFN_test pfn_test, char* desc);

void testManagerRunTests();

#endif
