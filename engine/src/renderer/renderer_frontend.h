#ifndef __RENDERER_FRONTEND_H__
#define __RENDERER_FRONTEND_H__

#include "./renderer_types.inl"

b8 rendererSystemInitialize(u64 *memoryRequirement, void *state, const char *applicationName);
void rendererSystemShutdown(void *state);

void rendererOnResized(u16 width, u16 height);

b8 rendererDrawFrame(RenderPacket *packet);

#endif
