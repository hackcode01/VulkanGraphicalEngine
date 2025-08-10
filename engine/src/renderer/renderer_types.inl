#ifndef __RENDERER_TYPES_INL__
#define __RENDERER_TYPES_INL__

#include "../defines.h"

typedef enum RendererBackendType {
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX
} RendererBackendType;

typedef struct RendererBackend {
    struct PlatformState* platformState;
    u64 frameNumber;

    b8 (*initialize)(struct RendererBackend* backend, const char* applicationName,
        struct PlatformState* platformState);

    void (*shutdown)(struct RendererBackend* backend);

    void (*resized)(struct RendererBackend* backend, u16 width, u16 height);

    b8 (*beginFrame)(struct RendererBackend* backend, f32 deltaTime);
    b8 (*endFrame)(struct RendererBackend* backend, f32 deltaTime);
} RendererBackend;

typedef struct RenderPacket {
    f32 deltaTime;
} RenderPacket;

#endif
