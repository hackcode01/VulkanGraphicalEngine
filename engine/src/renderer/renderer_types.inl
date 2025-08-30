#ifndef __RENDERER_TYPES_INL__
#define __RENDERER_TYPES_INL__

#include "../defines.h"
#include "../engine_math/math_types.h"

typedef enum RendererBackendType {
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX
} RendererBackendType;

typedef struct GlobalUniformObject {
    mat4 projection;
    mat4 view;
    mat4 m_reserved_1;
    mat4 m_reserved_2;
} GlobalUniformObject;

typedef struct RendererBackend {
    u64 frameNumber;

    b8 (*initialize)(struct RendererBackend *backend, const char *applicationName);

    void (*shutdown)(struct RendererBackend* backend);

    void (*resized)(struct RendererBackend* backend, u16 width, u16 height);

    void (*updateGlobalState)(mat4 projection, mat4 view, vec3 viewPosition,
        vec4 ambientColour, i32 mode);

    b8 (*beginFrame)(struct RendererBackend* backend, f32 deltaTime);
    b8 (*endFrame)(struct RendererBackend* backend, f32 deltaTime);

    void (*updateObject)(mat4 model);
} RendererBackend;

typedef struct RenderPacket {
    f32 deltaTime;
} RenderPacket;

#endif
