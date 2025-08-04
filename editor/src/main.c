#include "../../engine/src/core/logger.h"
#include "../../engine/src/core/asserts.h"

int main(void) {
    ENGINE_FATAL("A test message: %f", 3.14f);
    ENGINE_ERROR("A test message: %f", 3.14f);
    ENGINE_WARNING("A test message: %f", 3.14f);
    ENGINE_INFO("A test message: %f", 3.14f);
    ENGINE_DEBUG("A test message: %f", 3.14f);
    ENGINE_TRACE("A test message: %f", 3.14f);

    ENGINE_ASSERT(1 == 0);

    return EXIT_SUCCESS;
}
