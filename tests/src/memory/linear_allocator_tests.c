#include "../../include/memory/linear_allocator_tests.h"

#include "../../include/test_manager.h"
#include "../../include/expect.h"

#include "../../../engine/src/defines.h"

#include "../../../engine/src/engine_memory/linear_allocator.h"

u8 linearAllocatorShouldCreateAndDestroy() {
    LinearAllocator allocator;
    linearAllocatorCreate(sizeof(u64), 0, &allocator);

    expect_should_not_be(0, allocator.memory)
    expect_should_be(sizeof(u64), 0, &allocator)
    expect_should_be(0, allocator.allocated)

    linearAllocatorDestroy(&allocator);
}
