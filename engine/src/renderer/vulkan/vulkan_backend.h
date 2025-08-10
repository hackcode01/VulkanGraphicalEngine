#ifndef __VULKAN_BACKEND_H__
#define __VULKAN_BACKEND_H__

#include "../renderer_backend.h"

b8 vulkanRendererBackendInitialize(RendererBackend* backend,
    const char* applicationName,
    struct PlatformState* platformState
);

void vulkanRendererBackendShutdown(RendererBackend* backend);

void vulkanRendererBackendOnResize(RendererBackend* backend, u16 width, u16 height);

b8 vulkanRendererBackendBeginFrame(RendererBackend* backend, f32 deltaTime);
b8 vulkanRendererBackendEndFrame(RendererBackend* backend, f32 deltaTime);

#endif
