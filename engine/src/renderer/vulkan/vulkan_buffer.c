#include "vulkan_buffer.h"

#include "vulkan_device.h"
#include "vulkan_command_buffer.h"
#include "vulkan_utils.h"

#include "../../core/logger.h"
#include "../../engine_memory/engine_memory.h"

b8 vulkanBufferCreate(
    VulkanContext *context,
    u64 size,
    VkBufferUsageFlagBits usage,
    u32 memoryPropertyFlags,
    b8 bindOnCreate,
    VulkanBuffer *outBuffer) {
    engineZeroMemory(outBuffer, sizeof(VulkanBuffer));
    outBuffer->totalSize = size;
    outBuffer->usage = usage;
    outBuffer->memoryPropertyFlags = memoryPropertyFlags;

    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(context->device.logicalDevice, &bufferInfo, context->allocator, &outBuffer->handle))

    /** Gather memory requirements. */
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(context->device.logicalDevice, outBuffer->handle, &requirements);
    outBuffer->memoryIndex = context->findMemoryIndex(requirements.memoryTypeBits, outBuffer->memoryPropertyFlags);

    if (outBuffer->memoryIndex == -1) {
        ENGINE_ERROR("Unable to create vulkan buffer because the required memory type index was not found.");
        return false;
    }

    /** Allocate memory info. */
    VkMemoryAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocateInfo.allocationSize = requirements.size;
    allocateInfo.memoryTypeIndex = (u32)outBuffer->memoryIndex;

    /** Allocate the memory. */
    VkResult result = vkAllocateMemory(
        context->device.logicalDevice,
        &allocateInfo,
        context->allocator,
        &outBuffer->memory);

    if (result != VK_SUCCESS) {
        ENGINE_ERROR("Unable to create vulkan buffer because the required memory allocation failed. Error: %i", result);
        return false;
    }

    if (bindOnCreate) {
        vulkanBufferBind(context, outBuffer, 0);
    }

    return true;
}

void vulkanBufferDestroy(VulkanContext *context, VulkanBuffer *buffer) {
    if (buffer->memory) {
        vkFreeMemory(context->device.logicalDevice, buffer->memory, context->allocator);
        buffer->memory = 0;
    }

    if (buffer->handle) {
        vkDestroyBuffer(context->device.logicalDevice, buffer->handle, context->allocator);
        buffer->handle = 0;
    }

    buffer->totalSize = 0;
    buffer->usage = 0;
    buffer->isLocked = false;
}

b8 vulkanBufferResize(
    VulkanContext *context,
    u64 newSize,
    VulkanBuffer *buffer,
    VkQueue queue,
    VkCommandPool pool) {

    /** Create new buffer. */
    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = newSize;
    bufferInfo.usage = buffer->usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer newBuffer;
    VK_CHECK(vkCreateBuffer(context->device.logicalDevice, &bufferInfo, context->allocator, &newBuffer))

    /** Gather memory requirements. */
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(context->device.logicalDevice, newBuffer, &requirements);

    /** Allocate memory info. */
    VkMemoryAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocateInfo.allocationSize = requirements.size;
    allocateInfo.memoryTypeIndex = (u32)buffer->memoryIndex;

    /** Allocate the memory. */
    VkDeviceMemory newMemory;
    VkResult result = vkAllocateMemory(context->device.logicalDevice, &allocateInfo, context->allocator, &newMemory);

    if (result != VK_SUCCESS) {
        ENGINE_ERROR("Unable to resize vulkan buffer because the required memory allocation failed. Error: %i", result)
        return false;
    }

    /** Bind the new buffer's memory. */
    VK_CHECK(vkBindBufferMemory(context->device.logicalDevice, newBuffer, newMemory, 0))

    /** Copy over the data. */
    vulkanBufferCopyTo(context, pool, 0, queue, buffer->handle, 0, newBuffer, 0, buffer->totalSize);

    /** Make sure anything potentially using these is finished. */
    vkDeviceWaitIdle(context->device.logicalDevice);

    /** Destroy the old. */
    if (buffer->memory) {
        vkFreeMemory(context->device.logicalDevice, buffer->memory, context->allocator);
        buffer->memory = 0;
    }

    if (buffer->handle) {
        vkDestroyBuffer(context->device.logicalDevice, buffer->handle, context->allocator);
        buffer->handle = 0;
    }

    /** Set new properties. */
    buffer->totalSize = newSize;
    buffer->memory = newMemory;
    buffer->handle = newBuffer;

    return true;
}

void vulkanBufferBind(VulkanContext *context, VulkanBuffer *buffer, u64 offset) {
    VK_CHECK(vkBindBufferMemory(context->device.logicalDevice, buffer->handle, buffer->memory, offset))
}

void* vulkanBufferLockMemory(VulkanContext *context, VulkanBuffer *buffer, u64 offset, u64 size, u32 flags) {
    void *data;

    VK_CHECK(vkMapMemory(context->device.logicalDevice, buffer->memory, offset, size, flags, &data))
    return data;
}

void vulkanBufferUnlockMemory(VulkanContext *context, VulkanBuffer *buffer) {
    vkUnmapMemory(context->device.logicalDevice, buffer->memory);
}

void vulkanBufferLoadData(VulkanContext *context, VulkanBuffer *buffer, u64 offset,
    u64 size, u32 flags, const void *data) {
    void *dataPtr;
    VK_CHECK(vkMapMemory(context->device.logicalDevice, buffer->memory, offset, size, flags, &dataPtr))
    engineCopyMemory(dataPtr, data, size);
    vkUnmapMemory(context->device.logicalDevice, buffer->memory);
}

void vulkanBufferCopyTo(
    VulkanContext *context,
    VkCommandPool pool,
    VkFence fence,
    VkQueue queue,
    VkBuffer source,
    u64 sourceOffset,
    VkBuffer dest,
    u64 dest_offset,
    u64 size) {

    vkQueueWaitIdle(queue);

    /** Create a one-time-use command buffer. */
    VulkanCommandBuffer tempCommandBuffer;
    vulkanCommandBufferAllocateAndBeginSingleUse(context, pool, &tempCommandBuffer);

    /** Prepare the copy command and add it to the command buffer. */
    VkBufferCopy copyRegion;
    copyRegion.srcOffset = sourceOffset;
    copyRegion.dstOffset = dest_offset;
    copyRegion.size = size;

    vkCmdCopyBuffer(tempCommandBuffer.handle, source, dest, 1, &copyRegion);

    /** Submit the buffer for execution and wait for it to complete. */
    vulkanCommandBufferEndSingleUse(context, pool, &tempCommandBuffer, queue);
}
