#ifndef __RENDERER_FRONTEND_H__
#define __RENDERER_FRONTEND_H__

#include "./renderer_types.inl"

struct StaticMeshData;
struct PlatformState;

b8 rendererInitialize(const char* applicationName, struct PlatformState* platformState);
void rendererShutdown();

void rendererOnResized(u16 width, u16 height);

b8 rendererDrawFrame(RenderPacket* packet);

#endif
