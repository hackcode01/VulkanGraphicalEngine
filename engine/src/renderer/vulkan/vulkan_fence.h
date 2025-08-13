#ifndef __VULKAN_FENCE_H__
#define __VULKAN_FENCE_H__

#include "vulkan_types.inl"

void vulkanFenceCreate(
    VulkanContext* context,
    b8 createSignaled,
    VulkanFence* outFence
);

void vulkanFenceDestroy(VulkanContext* context, VulkanFence* fence);

b8 vulkanFenceWait(VulkanContext* context, VulkanFence* fence, u64 timeout_ns);

void vulkanFenceReset(VulkanContext* context, VulkanFence* fence);

#endif
