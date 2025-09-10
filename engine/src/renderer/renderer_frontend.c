#include "renderer_frontend.h"
#include "renderer_backend.h"

#include "../core/logger.h"
#include "../engine_memory/engine_memory.h"
#include "../engine_math/engine_math.h"

#include "../resources/resource_types.h"

typedef struct RendererSystemState {
    RendererBackend backend;
    mat4 projection;
    mat4 view;
    f32 nearClip;
    f32 farClip;

    Texture defaultTexture;
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

    statePtr->nearClip = 0.1f;
    statePtr->farClip = 1000.0f;
    statePtr->projection = mat4_perspective(deg_to_rad(45.0f), 1280 / 720.0f, statePtr->nearClip, statePtr->farClip);

    statePtr->view = mat4_translation((vec3){0, 0, -30.0f});
    statePtr->view = mat4_inverse(statePtr->view);

    /**
     * Create default texture, a 256x256 blue/white checkerboard pattern.
     * This is done in code to eliminate asset dependencies.
     */
    ENGINE_TRACE("Creating default texture...")
    const u32 textureDimension = 256;
    const u32 channels = 4;
    const u32 pixelCount = textureDimension * textureDimension;
    u8 pixels[262144];
    engineSetMemory(pixels, 255, sizeof(u8) * pixelCount * channels);

    /** Each pixels. */
    for (u64 row = 0; row < textureDimension; ++row) {
        for (u64 column = 0; column < textureDimension; ++column) {
            u64 index = (row * textureDimension) + column;
            u64 indexBpp = index * channels;

            if (row % 2) {
                if (column % 2) {
                    pixels[indexBpp + 0] = 0;
                    pixels[indexBpp + 1] = 0;
                }
            } else {
                if (!(column % 2)) {
                    pixels[indexBpp + 0] = 0;
                    pixels[indexBpp + 1] = 0;
                }
            }
        }
    }

    rendererCreateTexture("default", false, textureDimension, textureDimension, 4,
        pixels, false, &statePtr->defaultTexture);

    return true;
}

void rendererSystemShutdown(void *state) {
    if (statePtr) {
        rendererDestroyTexture(&statePtr->defaultTexture);

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
        statePtr->projection = mat4_perspective(deg_to_rad(45.0f), width / (f32)height,
                                                statePtr->nearClip, statePtr->farClip);
        statePtr->backend.resized(&statePtr->backend, width, height);
    } else {
        ENGINE_WARNING("Renderer backend does not exist to accept resize: %i %i",
            width, height)
    }
}

b8 rendererDrawFrame(RenderPacket* packet) {
    /** If the begin frame returned successfully, mid-frame operations may continue. */
    if (rendererBeginFrame(packet->deltaTime)) {
        statePtr->backend.updateGlobalState(statePtr->projection, statePtr->view,
            vec3_zero(), vec4_one(), 0);

        mat4 model = mat4_translation((vec3){0, 0, 0});
        /**
         * Static f32 angle = 0.01f;
         * angle += 0.001f;
         * quat rotation = quat_from_axis_angle(vec3_forward(), angle, false);
         * mat4 model = quat_to_rotation_matrix(rotation, vec3_zero());
         */
        GeometryRenderData data = {};
        data.objectID = 0;
        data.model = model;
        data.textures[0] = &statePtr->defaultTexture;
        statePtr->backend.updateObject(data);

        /** End the frame. If this fails, it's likely unrecoverable. */
        b8 result = rendererEndFrame(packet->deltaTime);

        if (!result) {
            ENGINE_ERROR("rendererEndFrame failed. Application shutting down...")
            return false;
        }
    }

    return true;
}

void rendererSetView(mat4 view) {
    statePtr->view = view;
}

void rendererCreateTexture(
    const char *name,
    b8 autoRelease,
    i32 width,
    i32 height,
    i32 channelCount,
    const u8 *pixels,
    b8 hasTransparency,
    struct Texture *outTexture) {

    statePtr->backend.createTexture(name, autoRelease, width, height, channelCount,
        pixels, hasTransparency, outTexture);
}

void rendererDestroyTexture(struct Texture *texture) {
    statePtr->backend.destroyTexture(texture);
}
