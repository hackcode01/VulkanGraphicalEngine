#include "engine_math.h"
#include "../platform/platform.h"

#include <math.h>
#include <stdlib.h>

static b8 randSeeded = FALSE;

/**
 * Note that these are here in order to prevent having to import the
 * entire <math.h> everywhere.
 */
f32 engine_sin(f32 x) {
    return sinf(x);
}

f32 engine_cos(f32 x) {
    return cosf(x);
}

f32 engine_tan(f32 x) {
    return tanf(x);
}

f32 engine_acos(f32 x) {
    return acosf(x);
}

f32 engine_sqrt(f32 x) {
    return sqrtf(x);
}

f32 engine_abs(f32 x) {
    return fabsf(x);
}

i32 engineRandom() {
    if (!randSeeded) {
        srand((u32)platformGetAbsoluteTime());
        randSeeded = TRUE;
    }

    return rand();
}

i32 engineRandomInRange(i32 min, i32 max) {
    if (!randSeeded) {
        srand((u32)platformGetAbsoluteTime());
        randSeeded = TRUE;
    }

    return (rand() % (max - min + 1)) + min;
}

f32 fEngineRandom() {
    return (float)engineRandom() / (f32)RAND_MAX;
}

f32 fEngineRandomInRange(f32 min, f32 max) {
    return min + ((float)engineRandom() / ((f32)RAND_MAX / (max - min)));
}
