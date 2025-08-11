#ifndef __VULKAN_SWAPCHAIN_H__
#define __VULKAN_SWAPCHAIN_H__

#include "vulkan_types.inl"

void vulkanSwapchainCreate(
    VulkanContext* context,
    u32 width,
    u32 height,
    VulkanSwapchain* outSwapchain
);

void vulkanSwapchainRecreate(
    VulkanContext* context,
    u32 width,
    u32 height,
    VulkanSwapchain* swapchain
);

void vulkanSwapchainDestroy(
    VulkanContext* context,
    VulkanSwapchain* swapchain
);

b8 vulkanSwapchainAcquireNextImageIndex(
    VulkanContext* context,
    VulkanSwapchain* swapchain,
    u64 timeout_ns,
    VkSemaphore imageAvailableSemaphore,
    VkFence fence,
    u32* outImageIndex
);

void vulkanSwapchainPresent(
    VulkanContext* context,
    VulkanSwapchain* swapchain,
    VkQueue graphicsQueue,
    VkQueue presentQueue,
    VkSemaphore renderCompleteSemaphore,
    u32 presentImageIndex
);

#endif
