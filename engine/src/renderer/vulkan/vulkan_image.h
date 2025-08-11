#ifndef __VULKAN_IMAGE_H__
#define __VULKAN_IMAGE_H__

#include "vulkan_types.inl"

void vulkanImageCreate(
    VulkanContext* context,
    VkImageType imageType,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memoryFlags,
    b32 createView,
    VkImageAspectFlags viewAspectFlags,
    VulkanImage* outImage
);

void vulkanImageViewCreate(
    VulkanContext* context,
    VkFormat format,
    VulkanImage* image,
    VkImageAspectFlags aspectFlags
);

void vulkanImageDestroy(VulkanContext* context, VulkanImage* image);

#endif
