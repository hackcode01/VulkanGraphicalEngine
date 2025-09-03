#ifndef __ENGINE_CLOCK_H__
#define __ENGINE_CLOCK_H__

#include "../defines.h"

typedef struct Clock {
    f64 startTime;
    f64 elapsed;
} Clock;

/**
 * Updated the provided clock. Should be called just before checking elapsed time.
 * Has no effect on non-started clocks.
 */
ENGINE_API void clockUpdate(Clock *clock);

/** Starts the provided clock. Resets elapsed time. */
ENGINE_API void clockStart(Clock *clock);

/** Stops the provided clock. Does not reset elapsed time. */
ENGINE_API void clockStop(Clock *clock);

#endif
