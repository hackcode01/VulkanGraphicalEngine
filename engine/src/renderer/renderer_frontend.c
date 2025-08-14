#include "renderer_frontend.h"
#include "renderer_backend.h"

#include "../core/logger.h"
#include "../engine_memory/engine_memory.h"

struct PlatformState;

/** Backend render context. */
static RendererBackend* backend = 0;

b8 rendererInitialize(const char* applicationName,
    struct PlatformState* platformState) {
    backend = engineAllocate(sizeof(RendererBackend), MEMORY_TAG_RENDERER);

    /** Make this configurable. */
    rendererBackendCreate(RENDERER_BACKEND_TYPE_VULKAN, platformState, backend);
    backend->frameNumber = 0;

    if (!backend->initialize(backend, applicationName, platformState)) {
        ENGINE_FATAL("Renderer backend failed to initialize. Shutting down.")
        return FALSE;
    }

    return TRUE;
}

void rendererShutdown() {
    backend->shutdown(backend);
    engineFree(backend, sizeof(RendererBackend), MEMORY_TAG_RENDERER);
}

b8 rendererBeginFrame(f32 deltaTime) {
    return backend->beginFrame(backend, deltaTime);
}

b8 rendererEndFrame(f32 deltaTime) {
    b8 result = backend->endFrame(backend, deltaTime);
    ++(backend->frameNumber);

    return result;
}

void rendererOnResized(u16 width, u16 height) {
    if (backend) {
        backend->resized(backend, width, height);
    } else {
        ENGINE_WARNING("Renderer backend does not exist to accept resize: %i %i",
            width, height)
    }
}

b8 rendererDrawFrame(RenderPacket* packet) {
    /** If the begin frame returned successfully, mid-frame operations may continue. */
    if (rendererBeginFrame(packet->deltaTime)) {
        /** End the frame. If this fails, it's likely unrecoverable. */
        b8 result = rendererEndFrame(packet->deltaTime);

        if (!result) {
            ENGINE_ERROR("rendererEndFrame failed. Application shutting down...")
            return FALSE;
        }
    }

    return TRUE;
}
