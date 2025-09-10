#ifndef __MATH_TYPES_H__
#define __MATH_TYPES_H__

#include "../defines.h"

typedef union vec2_u {
    /** An array of x, y. */
    f32 elements[2];
    struct {
        union {
            /** The first element. */
            f32 x;
            f32 r;
            f32 s;
            f32 u;
        };

        union {
            /** The second element. */
            f32 y;
            f32 g;
            f32 t;
            f32 v;
        };
    };
} vec2;

typedef struct vec3_u {
    union {
        /** An array of x, y, z. */
        f32 elements[3];
        struct {
            union {
                /** The first element. */
                f32 x;
                f32 r;
                f32 s;
                f32 u;
            };

            union {
                /** The second element. */
                f32 y;
                f32 g;
                f32 t;
                f32 v;
            };

            union {
                /** The third element. */
                f32 z;
                f32 b;
                f32 p;
                f32 w;
            };
        };
    };
} vec3;

typedef union vec4_u {

    /** An array of x, y, z, w. */
    f32 elements[4];
    union {
        struct {
            union {
                /** The first element. */
                f32 x;
                f32 r;
                f32 s;
            };

            union {
                /** The second element. */
                f32 y;
                f32 g;
                f32 t;
            };

            union {
                /** The third element. */
                f32 z;
                f32 b;
                f32 p;
            };

            union {
                /** The fourth element. */
                f32 w;
                f32 a;
                f32 q;
            };
        };
    };
} vec4;

typedef vec4 quat;

typedef union mat4_u {
    f32 data[16];
} mat4;

typedef struct vertex_3d {
    vec3 position;
    vec2 textureCoord;
} vertex_3d;

#endif
