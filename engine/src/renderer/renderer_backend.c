#include "./vulkan/vulkan_backend.h"

b8 rendererBackendCreate(RendererBackendType type,
    struct PlatformState* platformState,
    RendererBackend* outRendererBackend) {
    outRendererBackend->platformState = platformState;

    if (type == RENDERER_BACKEND_TYPE_VULKAN) {
        outRendererBackend->initialize = vulkanRendererBackendInitialize;
        outRendererBackend->shutdown = vulkanRendererBackendShutdown;
        outRendererBackend->beginFrame = vulkanRendererBackendBeginFrame;
        outRendererBackend->endFrame = vulkanRendererBackendEndFrame;
        outRendererBackend->resized = vulkanRendererBackendOnResize;

        return true;
    }

    return false;
}

void rendererBackendDestroy(RendererBackend* rendererBackend) {
    rendererBackend->initialize = 0;
    rendererBackend->shutdown = 0;
    rendererBackend->beginFrame = 0;
    rendererBackend->endFrame = 0;
    rendererBackend->resized = 0;
}
