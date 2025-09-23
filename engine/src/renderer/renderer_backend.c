#include "./vulkan/vulkan_backend.h"

b8 rendererBackendCreate(RendererBackendType type, RendererBackend *outRendererBackend) {
    if (type == RENDERER_BACKEND_TYPE_VULKAN) {
        outRendererBackend->initialize = vulkanRendererBackendInitialize;
        outRendererBackend->shutdown = vulkanRendererBackendShutdown;
        outRendererBackend->beginFrame = vulkanRendererBackendBeginFrame;
        outRendererBackend->updateGlobalState = vulkanRendererUpdateGlobalState;
        outRendererBackend->endFrame = vulkanRendererBackendEndFrame;
        outRendererBackend->resized = vulkanRendererBackendOnResize;
        outRendererBackend->updateObject = vulkanBackendUpdateObject;
        outRendererBackend->createTexture = vulkanRendererCreateTexture;
        outRendererBackend->destroyTexture = vulkanRendererDestroyTexture;
        outRendererBackend->createMaterial = vulkanRendererCreateMaterial;
        outRendererBackend->destroyMaterial = vulkanRendererDestroyMaterial;

        return true;
    }

    return false;
}

void rendererBackendDestroy(RendererBackend* rendererBackend) {
    rendererBackend->initialize = 0;
    rendererBackend->shutdown = 0;
    rendererBackend->beginFrame = 0;
    rendererBackend->updateGlobalState = 0;
    rendererBackend->endFrame = 0;
    rendererBackend->resized = 0;
    rendererBackend->updateObject = 0;
    rendererBackend->createTexture = 0;
    rendererBackend->destroyTexture = 0;
    rendererBackend->createMaterial = 0;
    rendererBackend->destroyMaterial = 0;
}
