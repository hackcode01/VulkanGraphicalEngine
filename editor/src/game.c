#include "game.h"

#include "../../engine/src/core/logger.h"
#include "../../engine/src/engine_memory/engine_memory.h"

#include "../../engine/src/core/input.h"

b8 gameInitialize(Game* gameInstance) {
    ENGINE_DEBUG("gameInitialize() called!")

    return true;
}

b8 gameUpdate(Game* gameInstance, f32 deltaTime) {
    static u64 allocationCount = 0;
    u64 prevAllocationCount = allocationCount;
    allocationCount = getMemoryAllocationCount();

    if (inputIsKeyUp('M') && inputWasKeyDown('M')) {
        ENGINE_DEBUG("Allocations: %llu (%llu this frame)",
            allocationCount, allocationCount - prevAllocationCount);
    }

    return true;
}

b8 gameRender(Game* gameInstance, f32 deltaTime) {
    return true;
}

void gameOnResize(Game* gameInstance, u32 width, u32 height) {}
