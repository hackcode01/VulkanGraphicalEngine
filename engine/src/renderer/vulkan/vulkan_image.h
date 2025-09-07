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

/**
 * Transitions the provided image from oldLayout to newLayout.
 */
void vulkanImageTransitionLayout(
    VulkanContext *context,
    VulkanCommandBuffer *commandBuffer,
    VulkanImage *image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout);

/**
 * Copies data in buffer to provided image.
 * @param context The Vulkan context.
 * @param image The image to copy the buffer's data to.
 * @param buffer The buffer whose data will be copied.
 */
void vulkanImageCopyFromBuffer(
    VulkanContext *context,
    VulkanImage *image,
    VkBuffer buffer,
    VulkanCommandBuffer *commandBuffer);

void vulkanImageDestroy(VulkanContext* context, VulkanImage* image);

#endif
