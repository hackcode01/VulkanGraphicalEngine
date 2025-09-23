#ifndef __ENGINE_VULKAN_BACKEND_H__
#define __ENGINE_VULKAN_BACKEND_H__

#include "../renderer_backend.h"

#include "../../resources/resource_types.h"

b8 vulkanRendererBackendInitialize(RendererBackend *backend, const char *applicationName);

void vulkanRendererBackendShutdown(RendererBackend* backend);

void vulkanRendererBackendOnResize(RendererBackend* backend, u16 width, u16 height);

b8 vulkanRendererBackendBeginFrame(RendererBackend* backend, f32 deltaTime);

void vulkanRendererUpdateGlobalState(mat4 projection, mat4 view, vec3 viewPosition,
    vec4 ambientColour, i32 mode);

b8 vulkanRendererBackendEndFrame(RendererBackend* backend, f32 deltaTime);

void vulkanBackendUpdateObject(GeometryRenderData data);

void vulkanRendererCreateTexture(const u8 *pixels, Texture *texture);
void vulkanRendererDestroyTexture(Texture *texture);

b8 vulkanRendererCreateMaterial(struct Material *material);
void vulkanRendererDestroyMaterial(struct Material *material);

#endif
