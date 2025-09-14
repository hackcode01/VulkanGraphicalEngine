#include "texture_system.h"

#include "../core/logger.h"

#include "../engine_memory/engine_string.h"
#include "../engine_memory/engine_memory.h"

#include "../containers/hashtable.h"

#include "../renderer/renderer_frontend.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/stb_image.h"

typedef struct TextureSystemState {
    TextureSystemConfig config;
    Texture defaultTexture;

    /** Array of registered textures. */
    Texture *registeredTextures;

    /** Hashtable for texture lookups. */
    Hashtable registeredTextureTable;
} TextureSystemState;

typedef struct TextureReference {
    u64 referenceCount;
    u32 handle;
    b8 autoRelease;
} TextureReference;

static TextureSystemState *statePtr = 0;

b8 createDefaultTextures(TextureSystemState *state);
void destroyDefaultTextures(TextureSystemState *state);
b8 loadTexture(const char *textureName, Texture *texture);

b8 textureSystemInitialize(u64 *memoryRequirement, void *state, TextureSystemConfig config) {
    if (config.maxTextureCount == 0) {
        ENGINE_FATAL("textureSystemInitialize - config.maxTextureCount must be > 0.")
        return false;
    }

    /** Block of memory will contain state structure, then block for array, then block for hashtable. */
    u64 structRequirement = sizeof(TextureSystemState);
    u64 arrayRequirement = sizeof(Texture) * config.maxTextureCount;
    u64 hashtableRequirement = sizeof(TextureReference) * config.maxTextureCount;

    *memoryRequirement = structRequirement + arrayRequirement + hashtableRequirement;

    if (!state) {
        return true;
    }

    statePtr = state;
    statePtr->config = config;

    void *arrayBlock = state + structRequirement;
    statePtr->registeredTextures = arrayBlock;

    void *hashtableBlock = arrayBlock + arrayRequirement;

    hashtableCreate(sizeof(TextureReference), config.maxTextureCount, hashtableBlock,
                    false, &statePtr->registeredTextureTable);

    TextureReference invalidRef;
    invalidRef.autoRelease = false;
    invalidRef.handle = INVALID_ID;
    invalidRef.referenceCount = 0;
    hashtableFill(&statePtr->registeredTextureTable, &invalidRef);

    u32 count = statePtr->config.maxTextureCount;
    for (u32 i = 0; i < count; ++i) {
        statePtr->registeredTextures[i].id = INVALID_ID;
        statePtr->registeredTextures[i].generation = INVALID_ID;
    }

    createDefaultTextures(statePtr);

    return true;
}

void textureSystemShutdown(void *state) {
    if (statePtr) {
        for (u32 i = 0; i < statePtr->config.maxTextureCount; ++i) {
            Texture *texture = &statePtr->registeredTextures[i];
            if (texture->generation != INVALID_ID) {
                rendererDestroyTexture(texture);
            }
        }

        destroyDefaultTextures(statePtr);

        statePtr = 0;
    }
}

Texture *textureSystemAcquire(const char *name, b8 autoRelease) {
    if (stringsEquali(name, DEFAULT_TEXTURE_NAME)) {
        ENGINE_WARNING("textureSystemAcquire called for default texture. "
            "Use textureSystemGetDefaultTexture for texture 'default'.")
        return &statePtr->defaultTexture;
    }

    TextureReference ref;
    if (statePtr && hashtableGet(&statePtr->registeredTextureTable, name, &ref)) {
        if (ref.referenceCount == 0) {
            ref.autoRelease = autoRelease;
        }
        ref.referenceCount++;

        if (ref.handle == INVALID_ID) {
            u32 count = statePtr->config.maxTextureCount;
            Texture *texture = 0;

            for (u32 i = 0; i < count; ++i) {
                if (statePtr->registeredTextures[i].id == INVALID_ID) {
                    ref.handle = i;
                    texture = &statePtr->registeredTextures[i];
                    break;
                }
            }

            if (!texture || ref.handle == INVALID_ID) {
                ENGINE_FATAL("textureSystemAcquire - Texture system cannot hold anymore textures. "
                    "Adjust configuration to allow more.")
                return 0;
            }

            if (!loadTexture(name, texture)) {
                ENGINE_ERROR("Failed to load texture '%s'.", name)
                return 0;
            }

            texture->id = ref.handle;
            ENGINE_TRACE("Texture '%s' does not yet exist. Created, and refCount is now %i.",
                name, ref.referenceCount)
        } else {
            ENGINE_TRACE("Texture '%s' already exists, refCount increased to %i.",
                name, ref.referenceCount)
        }

        hashtableSet(&statePtr->registeredTextureTable, name, &ref);

        return &statePtr->registeredTextures[ref.handle];
    }

    ENGINE_ERROR("texture_system_acquire failed to acquire texture '%s'. "
        "Null pointer will be returned.", name)
    return 0;
}

void textureSystemRelease(const char *name) {
    if (stringsEquali(name, DEFAULT_TEXTURE_NAME)) {
        return;
    }

    TextureReference ref;
    if (statePtr && hashtableGet(&statePtr->registeredTextureTable, name, &ref)) {
        if (ref.referenceCount == 0) {
            ENGINE_WARNING("Tried to release non-existent texture: '%s'", name)
            return;
        }

        ref.referenceCount--;
        if (ref.referenceCount == 0 && ref.autoRelease) {
            Texture *texture = &statePtr->registeredTextures[ref.handle];

            rendererDestroyTexture(texture);

            engineZeroMemory(texture, sizeof(Texture));
            texture->id = INVALID_ID;
            texture->generation = INVALID_ID;

            ref.handle = INVALID_ID;
            ref.autoRelease = false;
            ENGINE_TRACE("Released texture '%s'., "
                "Texture unloaded because reference count=0 and auto_release=true.", name)
        } else {
            ENGINE_TRACE("Released texture '%s', now has a reference count of '%i' (auto_release=%s).",
                name, ref.referenceCount, ref.autoRelease ? "true" : "false")
        }

        hashtableSet(&statePtr->registeredTextureTable, name, &ref);
    } else {
        ENGINE_ERROR("texture_system_release failed to release texture '%s'.", name)
    }
}

Texture *textureSystemGetDefaultTexture() {
    if (statePtr) {
        return &statePtr->defaultTexture;
    }

    ENGINE_ERROR("textureSystemGetDefaultTexture called before texture system initialization! "
        "Null pointer returned.")

    return 0;
}

b8 createDefaultTextures(TextureSystemState *state) {
    ENGINE_TRACE("Creating default texture...")

    const u32 textureDimension = 256;
    const u32 channels = 4;
    const u32 pixelCount = textureDimension * textureDimension;
    u8 pixels[262144];
    engineSetMemory(pixels, 255, sizeof(u8) * pixelCount * channels);

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

    rendererCreateTexture(DEFAULT_TEXTURE_NAME, textureDimension, textureDimension,
        4, pixels, false, &state->defaultTexture);
    state->defaultTexture.generation = INVALID_ID;

    return true;
}

void destroyDefaultTextures(TextureSystemState *state) {
    if (state) {
        rendererDestroyTexture(&state->defaultTexture);
    }
}

b8 loadTexture(const char *textureName, Texture *texture) {
    char *formatStr = "assets/textures/%s.%s";
    const i32 requiredChannelCount = 4;
    stbi_set_flip_vertically_on_load(true);
    char fullFilePath[512];

    stringFormat(fullFilePath, formatStr, textureName, "png");

    Texture tempTexture;

    u8* data = stbi_load(
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
            u8 a = data[i + 3];
            if (a < 255) {
                hasTransparency = true;
                break;
            }
        }

        if (stbi_failure_reason()) {
            ENGINE_WARNING("loadTexture() failed to load file '%s': %s",
                fullFilePath, stbi_failure_reason())
        }

        rendererCreateTexture(
            textureName,
            tempTexture.width,
            tempTexture.height,
            tempTexture.channelCount,
            data,
            hasTransparency,
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
            ENGINE_WARNING("loadTexture() failed to load file '%s': %s",
                fullFilePath, stbi_failure_reason())
        }

        return false;
    }
}
