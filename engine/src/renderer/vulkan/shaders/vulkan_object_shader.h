#ifndef __VULKAN_OBJECT_SHADER_H__
#define __VULKAN_OBJECT_SHADER_H__

#include "../vulkan_types.inl"
#include "../../../renderer/renderer_types.inl"

b8 vulkanObjectShaderCreate(VulkanContext *context, VulkanObjectShader *outShader);

void vulkanObjectShaderDestroy(VulkanContext *context, struct VulkanObjectShader *shader);

void vulkanObjectShaderUse(VulkanContext *context, struct VulkanObjectShader *shader);

void vulkanObjectShaderUpdateGlobalState(VulkanContext *context,
    struct VulkanObjectShader *shader);

void vulkanObjectShaderUpdateObject(VulkanContext *context,
    struct VulkanObjectShader *shader, mat4 model);

#endif
