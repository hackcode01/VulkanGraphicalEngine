#ifndef __ENGINE_VULKAN_PIPELINE_H__
#define __ENGINE_VULKAN_PIPELINE_H__

#include "vulkan_types.inl"

b8 vulkanGraphicsPipelineCreate(
    VulkanContext *context,
    VulkanRenderpass *renderpass,
    u32 attributeCount,
    VkVertexInputAttributeDescription *attributes,
    u32 descriptorSetLayoutCount,
    VkDescriptorSetLayout *descriptorSetLayouts,
    u32 stageCount,
    VkPipelineShaderStageCreateInfo *stages,
    VkViewport viewport,
    VkRect2D scissor,
    b8 isWireframe,
    VulkanPipeline *outPipeline
);

void vulkanPipelineDestroy(VulkanContext *context, VulkanPipeline *pipeline);

void vulkanPipelineBind(VulkanCommandBuffer *commandBuffer, VkPipelineBindPoint bindPoint,
    VulkanPipeline *pipeline);

#endif
