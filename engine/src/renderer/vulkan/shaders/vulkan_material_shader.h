#ifndef __VULKAN_OBJECT_SHADER_H__
#define __VULKAN_OBJECT_SHADER_H__

#include "../vulkan_types.inl"
#include "../../../renderer/renderer_types.inl"

b8 vulkanMaterialShaderCreate(VulkanContext *context, VulkanMaterialShader *outShader);

void vulkanMaterialShaderDestroy(VulkanContext *context, struct VulkanMaterialShader *shader);

void vulkanMaterialShaderUse(VulkanContext *context, struct VulkanMaterialShader *shader);

void vulkanMaterialShaderUpdateGlobalState(VulkanContext *context,
    struct VulkanMaterialShader *shader, f32 deltaTime);

void vulkanMaterialShaderUpdateObject(VulkanContext *context,
    struct VulkanMaterialShader *shader, GeometryRenderData data);

b8 vulkanMaterialShaderAcquireResources(VulkanContext *context,
    struct VulkanMaterialShader *shader, u32 *outObjectID);

void vulkanMaterialShaderReleaseResources(VulkanContext *context,
    struct VulkanMaterialShader *shader, u32 objectID);

#endif
