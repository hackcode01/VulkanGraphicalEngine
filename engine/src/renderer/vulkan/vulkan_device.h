#ifndef __VULKAN_DEVICE_H__
#define __VULKAN_DEVICE_H__

#include "vulkan_types.inl"

b8 vulkanDeviceCreate(VulkanContext* context);

void vulkanDeviceDestroy(VulkanContext* context);

void vulkanDeviceQuerySwapchainSupport(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,

    VulkanSwapchainSupportInfo* outSupportInfo
);

b8 vulkanDeviceDetectDepthFormat(VulkanDevice* device);

#endif
