#include "vulkan_command_buffer.h"

#include "../../engine_memory/engine_memory.h"

void vulkanCommandBufferAllocate(
    VulkanContext* context,
    VkCommandPool pool,
    b8 isPrimary,
    VulkanCommandBuffer* outCommandBuffer) {

    engineZeroMemory(outCommandBuffer, sizeof(outCommandBuffer));

    VkCommandBufferAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocateInfo.commandPool = pool;
    allocateInfo.level = isPrimary = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY :
                                                 VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.pNext = 0;

    outCommandBuffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
    VK_CHECK(vkAllocateCommandBuffers(
        context->device.logicalDevice,
        &allocateInfo,
        &outCommandBuffer->handle
    ))
    outCommandBuffer->state = COMMAND_BUFFER_STATE_READY;
}

void vulkanCommandBufferFree(
    VulkanContext* context,
    VkCommandPool pool,
    VulkanCommandBuffer* commandBuffer) {

    vkFreeCommandBuffers(
        context->device.logicalDevice,
        pool,
        1,
        &commandBuffer->handle
    );

    commandBuffer->handle = 0;
    commandBuffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}

void vulkanCommandBufferBegin(
    VulkanCommandBuffer* commandBuffer,
    b8 isSignleUse,
    b8 isRenderPassContinue,
    b8 isSimultaneousUse) {

    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = 0;
    if (isSignleUse) {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }

    if (isRenderPassContinue) {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }

    if (isSimultaneousUse) {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    }

    VK_CHECK(vkBeginCommandBuffer(commandBuffer->handle, &beginInfo))
    commandBuffer->state = COMMAND_BUFFER_STATE_RECORDING;
}

void vulkanCommandBufferEnd(VulkanCommandBuffer* commandBuffer) {
    VK_CHECK(vkEndCommandBuffer(commandBuffer->handle))
    commandBuffer->state = COMMAND_BUFFER_STATE_RECORDING_ENDED;
}

void vulkanCommandBufferUpdateSubmitted(VulkanCommandBuffer* commandBuffer) {
    commandBuffer->state = COMMAND_BUFFER_STATE_SUBMITTED;
}

void vulkanCommandBufferReset(VulkanCommandBuffer* commandBuffer) {
    commandBuffer->state = COMMAND_BUFFER_STATE_READY;
}

/**
 * Allocates and begins recording to outCommandBuffer.
 */
void vulkanCommandBufferAllocateAndBeginSingleUse(
    VulkanContext* context,
    VkCommandPool pool,
    VulkanCommandBuffer* outCommandBuffer) {

    vulkanCommandBufferAllocate(context, pool, TRUE, outCommandBuffer);
    vulkanCommandBufferBegin(outCommandBuffer, TRUE, FALSE, FALSE);
}

/**
 * Ends recording, submits to and waits for queue operation and
 * frees the provided command buffer.
 */
void vulkanCommandBufferEndSingleUse(
    VulkanContext* context,
    VkCommandPool pool,
    VulkanCommandBuffer* commandBuffer,
    VkQueue queue) {

    /** End the command buffer. */
    vulkanCommandBufferEnd(commandBuffer);

    /** Submit the queue. */
    VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer->handle;

    VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, 0));

    /** Wait for it to finish. */
    VK_CHECK(vkQueueWaitIdle(queue));

    /** Free the command buffer. */
    vulkanCommandBufferFree(context, pool, commandBuffer);
}
