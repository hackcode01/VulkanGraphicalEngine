#ifndef __VULKAN_OBJECT_SHADER_H__
#define __VULKAN_OBJECT_SHADER_H__

#include "../vulkan_types.inl"
#include "../../../renderer/renderer_types.inl"

b8 vulkanObjectShaderCreate(VulkanContext *context, VulkanObjectShader *outShader);

void vulkanObjectShaderDestroy(VulkanContext *context, struct VulkanObjectShader *shader);

void vulkanObjectShaderUse(VulkanContext *context, struct VulkanObjectShader *shader);

void vulkanObjectShaderUpdateGlobalState(VulkanContext *context,
    struct VulkanObjectShader *shader, f32 deltaTime);

void vulkanObjectShaderUpdateObject(VulkanContext *context,
    struct VulkanObjectShader *shader, GeometryRenderData data);

b8 vulkanObjectShaderAcquireResources(VulkanContext *context,
    struct VulkanObjectShader *shader, u32 *outObjectID);

void vulkanObjectShaderReleaseResources(VulkanContext *context,
    struct VulkanObjectShader *shader, u32 objectID);

#endif
