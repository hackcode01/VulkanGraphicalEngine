#ifndef __VULKAN_RENDER_PASS_H__
#define __VULKAN_RENDER_PASS_H__

#include "vulkan_types.inl"

void vulkanRenderPassCreate(
    VulkanContext* context,
    VulkanRenderPass* outRenderpass,
    f32 coordinate_x,
    f32 coordinate_y,
    f32 width,
    f32 height,
    f32 red,
    f32 green,
    f32 blue,
    f32 alpha,
    f32 depth,
    u32 stencil
);

void vulkanRenderPassDestroy(VulkanContext* context, VulkanRenderPass* renderpass);

void vulkanRenderPassBegin(
    VulkanCommandBuffer* commandBuffer,
    VulkanRenderPass* renderPass,
    VkFramebuffer frameBuffer
);

void vulkanRenderPassEnd(VulkanCommandBuffer* commandBuffer, VulkanRenderPass* renderPass);

#endif
