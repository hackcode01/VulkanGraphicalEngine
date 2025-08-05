#include "../../engine/src/core/logger.h"
#include "../../engine/src/core/asserts.h"

#include "../../engine/src/platform/platform.h"

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

    PlatformState_t state;
    if (platformStartup(&state, "Engine Editor", 100, 100, 1800, 800)) {
        while (TRUE) {
            platformPumpMessages(&state);
        }
    }
    platformShutdown(&state);

    printf_s("\n\nEnter any key for continue...");

#ifdef _MSC_VER
    _Analysis_noreturn_ _getch();
#else
    _getch();
#endif

    return EXIT_SUCCESS;
}
