#include "../../engine/src/application/includes/application.h"
#include "../../engine/src/core/logger.h"
#include "../../engine/src/game_types.h"

#include "./entry.h"

#include "../../engine/src/core/memory/includes/memory.h"

int main(void) {
    initializeMemory();

    /* Request the game instance from the application. */
    game_t gameInstance;
    if (!createGame(&gameInstance)) {
        FATAL("Could not create game!");
        return -1;
    }

    /* Ensure the function pointers exist. */
    if (!gameInstance.render || !gameInstance.update || !gameInstance.initialize || !gameInstance.onResize) {
        FATAL("The game's function pointers must be assigned!");
        return -2;
    }

    /* Initialization. */
    if (!applicationCreate(&gameInstance)) {
        INFO("Application failed to create!.");
        return 1;
    }

    /* Begin the game loop. */
    if(!applicationRun()) {
        INFO("Application did not shutdown gracefully.");
        return 2;
    }

    shutdownMemory();

    return 0;
}
