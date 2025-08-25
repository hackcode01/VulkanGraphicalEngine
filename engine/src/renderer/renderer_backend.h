#ifndef __RENDERER_BACKEND_H__
#define __RENDERER_BACKEND_H__

#include "./renderer_types.inl"

struct PlatformState;

/** Create backend renderer. */
b8 rendererBackendCreate(RendererBackendType type, RendererBackend* outRendererbackend);

/** Destroy backend renderer. */
void rendererBackendDestroy(RendererBackend* rendererBackend);

#endif
