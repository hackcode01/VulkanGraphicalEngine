#include "../include/test_manager.h"
#include "../include/memory/linear_allocator_tests.h"
#include "../../engine/src/core/logger.h"

#include <stdlib.h>

int main(void) {
    /** Always initialize the test manager first. */
    testManagerInit();

    /** Add test registrations here. */
    linearAllocatorRegisterTests();

    ENGINE_DEBUG("Starting tests...\n")

    /** Execute tests. */
    testManagerRunTests();

    return EXIT_SUCCESS;
}
