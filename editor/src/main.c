#include "../../engine/src/core/logger.h"
#include "../../engine/src/core/asserts.h"

#include <stdlib.h>
#include <stdio.h>

#include <conio.h>

int main(void) {
    ENGINE_FATAL("A test message: %f", 3.14f);
    ENGINE_ERROR("A test message: %f", 3.14f);
    ENGINE_WARNING("A test message: %f", 3.14f);
    ENGINE_INFO("A test message: %f", 3.14f);
    ENGINE_DEBUG("A test message: %f", 3.14f);
    ENGINE_TRACE("A test message: %f", 3.14f);

    ENGINE_ASSERT(0 == 0);

    printf_s("\n\nEnter any key for continue...");

#ifdef _MSC_VER
    _Analysis_noreturn_ _getch();
#else
    _getch();
#endif

    return EXIT_SUCCESS;
}
