#include "renderer_frontend.h"
#include "renderer_backend.h"

#include "../core/logger.h"
#include "../engine_memory/engine_memory.h"
#include "../engine_math/engine_math.h"

#include "../resources/resource_types.h"

#include "../engine_memory/engine_string.h"
#include "../core/event.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/stb_image.h"

typedef struct RendererSystemState {
    RendererBackend backend;
    mat4 projection;
    mat4 view;
    f32 nearClip;
    f32 farClip;

    Texture defaultTexture;

    Texture testDiffuse;
} RendererSystemState;

static RendererSystemState *statePtr;

void createTexture(Texture *texture) {
    engineZeroMemory(texture, sizeof(Texture));
    texture->generation = INVALID_ID;
}

b8 loadTexture(const char *textureName, Texture *texture) {
    char *formatStr = "assets/textures%s.%s";
    const i32 requiredChannelCount = 4;
    stbi_set_flip_vertically_on_load(true);
    char fullFilePath[512];

    stringFormat(fullFilePath, formatStr, textureName, "png");

    Texture tempTexture;

    u8 *data = stbi_load(
        fullFilePath,
        (i32*)&tempTexture.width,
        (i32*)&tempTexture.height,
        (i32*)&tempTexture.channelCount,
        requiredChannelCount);

    tempTexture.channelCount = requiredChannelCount;

    if (data) {
        u32 currentGeneration = texture->generation;
        texture->generation = INVALID_ID;

        u64 totalSize = tempTexture.width * tempTexture.height * requiredChannelCount;
        b32 hasTransparency = false;
        for (u64 i = 0; i < totalSize; i += requiredChannelCount) {
            u8 alpha = data[i + 3];
            if (alpha < 255) {
                hasTransparency = true;
                break;
            }
        }

        if (stbi_failure_reason()) {
            ENGINE_WARNING("loadTexture() failed to load file '%s': %s", fullFilePath,
                stbi_failure_reason())
        }

        rendererCreateTexture(textureName, true, tempTexture.width,
            tempTexture.height, tempTexture.channelCount, data, hasTransparency,
            &tempTexture);

        Texture old = *texture;
        *texture = tempTexture;

        rendererDestroyTexture(&old);

        if (currentGeneration == INVALID_ID) {
            texture->generation = 0;
        } else {
            texture->generation = currentGeneration + 1;
        }

        stbi_image_free(data);
        return true;
    } else {
        if (stbi_failure_reason()) {
            ENGINE_WARNING("loadTexture() failed to load file '%s': %s", fullFilePath,
                stbi_failure_reason())
        }

        return false;
    }
}

b8 eventOnDebugEvent(u16 code, void *sender, void *listenerInstance, EventContext data) {
    const char *names[3] = {
        "cobblestone",
        "paving_1",
        "paving_2"
    };
    static i8 choice = 2;
    choice++;
    choice %= 3;

    loadTexture(names[choice], &statePtr->testDiffuse);

    return true;
}

b8 rendererSystemInitialize(u64 *memoryRequirement, void *state, const char *applicationName) {
    *memoryRequirement = sizeof(RendererSystemState);
    if (state == 0) {
        return true;
    }

    statePtr = state;

    eventRegister(EVENT_CODE_DEBUG_0, statePtr, eventOnDebugEvent);

    statePtr->backend.defaultDiffuse = &statePtr->defaultTexture;

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

    statePtr->defaultTexture.generation = INVALID_ID;

    createTexture(&statePtr->testDiffuse);

    return true;
}

void rendererSystemShutdown(void *state) {
    if (statePtr) {
        eventUnregister(EVENT_CODE_DEBUG_0, statePtr, eventOnDebugEvent);

        rendererDestroyTexture(&statePtr->defaultTexture);

        rendererDestroyTexture(&statePtr->testDiffuse);

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
        data.textures[0] = &statePtr->testDiffuse;
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
