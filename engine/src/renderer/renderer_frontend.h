#ifndef __RENDERER_FRONTEND_H__
#define __RENDERER_FRONTEND_H__

#include "./renderer_types.inl"

struct StaticMeshData;
struct PlatformState;

b8 rendererInitialize(const char* applicationName, struct PlatformState* platformState);
void rendererShutdown();

b8 rendererDrawFrame(RenderPacket* packet);

#endif
