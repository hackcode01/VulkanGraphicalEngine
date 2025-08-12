#ifndef __VULKAN_COMMAND_BUFFER_H__
#define __VULKAN_COMMAND_BUFFER_H__

#include "vulkan_types.inl"

void vulkanCommandBufferAllocate(
    VulkanContext* context,
    VkCommandPool pool,
    b8 isPrimary,
    VulkanCommandBuffer* outCommandBuffer
);

void vulkanCommandBufferFree(
    VulkanContext* context,
    VkCommandPool pool,
    VulkanCommandBuffer* commandBuffer
);

void vulkanCommandBufferBegin(
    VulkanCommandBuffer* commandBuffer,
    b8 isSignleUse,
    b8 isRenderPassContinue,
    b8 isSimultaneousUse
);

void vulkanCommandBufferEnd(VulkanCommandBuffer* commandBuffer);

void vulkanCommandBufferUpdateSubmitted(VulkanCommandBuffer* commandBuffer);

void vulkanCommandBufferReset(VulkanCommandBuffer* commandBuffer);

/**
 * Allocates and begins recording to outCommandBuffer.
 */
void vulkanCommandBufferAllocateAndBeginSingleUse(
    VulkanContext* context,
    VkCommandPool pool,
    VulkanCommandBuffer* outCommandBuffer
);

/**
 * Ends recording, submits to and waits for queue operation and
 * frees the provided command buffer.
 */
void vulkanCommandBufferEndSingleUse(
    VulkanContext* context,
    VkCommandPool pool,
    VulkanCommandBuffer* commandBuffer,
    VkQueue queue
);

#endif /** __VULKAN_COMMAND_BUFFER_H__ */
