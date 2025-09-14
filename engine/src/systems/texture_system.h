#ifndef __ENGINE_TEXTURE_SYSTEM_H__
#define __ENGINE_TEXTURE_SYSTEM_H__

#include "../renderer/renderer_types.inl"

typedef struct TextureSystemConfig {
    u32 maxTextureCount;
} TextureSystemConfig;

#define DEFAULT_TEXTURE_NAME "default"

b8 textureSystemInitialize(u64 *memoryRequirement, void *state,
    TextureSystemConfig config);
void textureSystemShutdown(void *state);

Texture *textureSystemAcquire(const char *name, b8 autoRelease);
void textureSystemRelease(const char *name);

Texture *textureSystemGetDefaultTexture();

#endif
