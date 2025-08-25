#include "renderer_frontend.h"
#include "renderer_backend.h"

#include "../core/logger.h"
#include "../engine_memory/engine_memory.h"

typedef struct RendererSystemState {
    RendererBackend backend;
} RendererSystemState;

static RendererSystemState *statePtr;

b8 rendererSystemInitialize(u64 *memoryRequirement, void *state, const char *applicationName) {
    *memoryRequirement = sizeof(RendererSystemState);
    if (state == 0) {
        return true;
    }

    statePtr = state;

    rendererBackendCreate(RENDERER_BACKEND_TYPE_VULKAN, &statePtr->backend);
    statePtr->backend.frameNumber = 0;

    if (!statePtr->backend.initialize(&statePtr->backend, applicationName)) {
        ENGINE_FATAL("Renderer backend failed to initialize. Shutting down.");
        return false;
    }

    return true;
}

void rendererSystemShutdown(void *state) {
    if (statePtr) {
        statePtr->backend.shutdown(&statePtr->backend);
    }
    statePtr = 0;
}

b8 rendererBeginFrame(f32 deltaTime) {
    if (!statePtr) {
        return false;
    }

    return statePtr->backend.beginFrame(&statePtr->backend, deltaTime);
}

b8 rendererEndFrame(f32 deltaTime) {
    if (!statePtr) {
        return false;
    }

    b8 result = statePtr->backend.endFrame(&statePtr->backend, deltaTime);
    statePtr->backend.frameNumber++;

    return result;
}

void rendererOnResized(u16 width, u16 height) {
    if (statePtr) {
        statePtr->backend.resized(&statePtr->backend, width, height);
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
            return false;
        }
    }

    return true;
}
