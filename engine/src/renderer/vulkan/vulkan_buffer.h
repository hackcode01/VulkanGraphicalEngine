#ifndef __ENGINE_VULKAN_BUFFER_H__
#define __ENGINE_VULKAN_BUFFER_H__

#include "vulkan_types.inl"

b8 vulkanBufferCreate(
    VulkanContext* context,
    u64 size,
    VkBufferUsageFlagBits usage,
    u32 memoryPropertyFlags,
    b8 bindOnCreate,
    VulkanBuffer *outBuffer);

void vulkanBufferDestroy(VulkanContext *context, VulkanBuffer *buffer);

b8 vulkanBufferResize(
    VulkanContext *context,
    u64 newSize,
    VulkanBuffer *buffer,
    VkQueue queue,
    VkCommandPool pool);

void vulkanBufferBind(VulkanContext *context, VulkanBuffer *buffer, u64 offset);

void *vulkanBufferLockMemory(VulkanContext *context, VulkanBuffer *buffer,
    u64 offset, u64 size, u32 flags);

void vulkanBufferUnlockMemory(VulkanContext *context, VulkanBuffer *buffer);

void vulkanBufferLoadData(VulkanContext *context, VulkanBuffer *buffer, u64 offset,
    u64 size, u32 flags, const void *data);

void vulkanBufferCopyTo(
    VulkanContext* context,
    VkCommandPool pool,
    VkFence fence,
    VkQueue queue,
    VkBuffer source,
    u64 source_offset,
    VkBuffer dest,
    u64 dest_offset,
    u64 size);

#endif
