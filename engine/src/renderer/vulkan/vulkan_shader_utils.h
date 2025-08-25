#ifndef __VULKAN_SHADER_UTILS_H__
#define __VULKAN_SHADER_UTILS_H__

#include "vulkan_types.inl"

b8 createShaderModule(
    VulkanContext *context,
    const char *name,
    const char *typeStr,
    VkShaderStageFlagBits shaderStageFlag,
    u32 stageIndex,
    VulkanShaderStage *shaderStage
);

#endif
