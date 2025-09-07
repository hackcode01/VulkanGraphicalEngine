#ifndef __RENDERER_FRONTEND_H__
#define __RENDERER_FRONTEND_H__

#include "./renderer_types.inl"

b8 rendererSystemInitialize(u64 *memoryRequirement, void *state, const char *applicationName);
void rendererSystemShutdown(void *state);

void rendererOnResized(u16 width, u16 height);

b8 rendererDrawFrame(RenderPacket *packet);

/** HACK: this should not be exposed outside the engine. */
ENGINE_API void rendererSetView(mat4 view);

void rendererCreateTexture(
    const char *name,
    b8 autoRelease,
    i32 width,
    i32 height,
    i32 channelCount,
    const u8 *pixels,
    b8 hasTransparency,
    struct Texture *outTexture);

void rendererDestroyTexture(struct Texture *texture);

#endif
