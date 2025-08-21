#include "../../include/memory/linear_allocator_tests.h"

#include "../../include/test_manager.h"
#include "../../include/expect.h"

#include "../../../engine/src/defines.h"

#include "../../../engine/src/engine_memory/linear_allocator.h"

u8 linearAllocatorShouldCreateAndDestroy() {
    LinearAllocator allocator = {};
    linearAllocatorCreate(sizeof(u64), 0, &allocator);

    expect_should_not_be(0, allocator.memory)
    expect_should_be(sizeof(u64), &allocator)
    expect_should_be(0, allocator.allocated)

    linearAllocatorDestroy(&allocator);

    expect_should_be(0, allocator.memory)
    expect_should_be(0, allocator.totalSize)
    expect_should_be(0, allocator.allocated)

    return true;
}

u8 linearAllocatorSingleAllocationAllSpace() {
    LinearAllocator allocator;
    linearAllocatorCreate(sizeof(u64), 0, &allocator);

    /** Single allocation. */
    void* block = linearAllocatorAllocate(&allocator, sizeof(u64));

    /** Validate it. */
    expect_should_be(0, block)
    expect_should_be(sizeof(u64), allocator.allocated)

    linearAllocatorDestroy(&allocator);

    return true;
}

u8 linearAllocatorMultiAllocationAllSpace() {
    u64 maxAllocs = 1024;
    LinearAllocator allocator = {};
    linearAllocatorCreate(sizeof(u64) * maxAllocs, 0, &allocator);

    /** Multiple allocations - full. */
    void* block = 0;
        for (u64 i = 0; i < maxAllocs; ++i) {
        block = linearAllocatorAllocate(&allocator, sizeof(u64));
        /** Validate it. */
        expect_should_not_be(0, block)
        expect_should_be(sizeof(u64) * (i + 1), allocator.allocated)
    }

    linearAllocatorDestroy(&allocator);

    return true;
}

u8 linearAllocatorMultiAllocationOverAllocate() {
    u64 maxAllocs = 3;
    LinearAllocator allocator = {};
    linearAllocatorCreate(sizeof(u64) * maxAllocs, 0, &allocator);

    /** Multiple allocations - full. */
    void* block = 0;
    for (u64 i = 0; i < maxAllocs; ++i) {
        block = linearAllocatorAllocate(&allocator, sizeof(u64));
        /** Validate it. */
        expect_should_not_be(0, block);
        expect_should_be(sizeof(u64) * (i + 1), allocator.allocated);
    }

    ENGINE_DEBUG("Note: The following error is intentionally caused by this test.");

    /** Ask for one more allocation. Should error and return 0. */
    block = linearAllocatorAllocate(&allocator, sizeof(u64));
    /** Validate it - allocated should be unchanged. */
    expect_should_be(0, block);
    expect_should_be(sizeof(u64) * (maxAllocs), allocator.allocated);

    linearAllocatorDestroy(&allocator);

    return true;
}

u8 linearAllocatorMultiAllocationAllSpaceThenFree() {
    u64 maxAllocs = 1024;
    LinearAllocator allocator = {};
    linearAllocatorCreate(sizeof(u64) * maxAllocs, 0, &allocator);

    /** Multiple allocations - full. */
    void* block = 0;
    for (u64 i = 0; i < maxAllocs; ++i) {
        block = linearAllocatorAllocate(&allocator, sizeof(u64));
        /** Validate it. */
        expect_should_not_be(0, block);
        expect_should_be(sizeof(u64) * (i + 1), allocator.allocated);
    }

    /** Validate that pointer is reset. */
    linearAllocatorFreeAll(&allocator);
    expect_should_be(0, allocator.allocated);

    linearAllocatorDestroy(&allocator);

    return true;
}

void linearAllocatorRegisterTests() {
    testManagerRegisterTest(linearAllocatorShouldCreateAndDestroy,
        "Linear allocator should create and destroy");
    testManagerRegisterTest(linearAllocatorSingleAllocationAllSpace,
        "Linear allocator single allocator for all space");
    testManagerRegisterTest(linearAllocatorMultiAllocationAllSpace,
        "Linear allocator multi allocator for all space");
    testManagerRegisterTest(linearAllocatorMultiAllocationOverAllocate,
        "Linear allocator try over allocate");
    testManagerRegisterTest(linearAllocatorMultiAllocationAllSpaceThenFree,
        "Linear allocator allocated should be 0 after free_all");
}
