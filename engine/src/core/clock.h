#ifndef __CLOCK_H__
#define __CLOCK_H__

#include "../defines.h"

typedef struct Clock {
    f64 startTime;
    f64 elapsed;
} Clock;

/**
 * Updated the provided clock. Should be called just before checking elapsed time.
 * Has no effect on non-started clocks.
 */
ENGINE_API clockUpdate(Clock* clock);

/** Starts the provided clock. Resets elapsed time. */
ENGINE_API clockStart(Clock* clock);

/** Stops the provided clock. Does not reset elapsed time. */
ENGINE_API clockStop(Clock* clock);

#endif
