#include "vulkan_image.h"

#include "vulkan_device.h"

#include "../../engine_memory/engine_memory.h"
#include "../../core/logger.h"

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
    VulkanImage* outImage) {

    /** Copy params */
    outImage->width = width;
    outImage->height = height;

    /** Creation info. */
    VkImageCreateInfo imageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 4;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = tiling;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = usage;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateImage(context->device.logicalDevice, &imageCreateInfo,
        context->allocator, &outImage->handle))

    /** Query memory requirements. */
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(context->device.logicalDevice, outImage->handle,
        &memoryRequirements);

    i32 memoryType = context->findMemoryIndex(memoryRequirements.memoryTypeBits,
        memoryFlags);
    if (memoryType == -1) {
        ENGINE_ERROR("Required memory type not found. Image not valid.")
    }

    /** Allocate memory. */
    VkMemoryAllocateInfo memoryAllocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = memoryType;
    VK_CHECK(vkAllocateMemory(context->device.logicalDevice, &memoryAllocateInfo,
        context->allocator, &outImage->memory))

    /** Bind the memory. */
    VK_CHECK(vkBindImageMemory(context->device.logicalDevice, outImage->handle, outImage->memory, 0))

    if (createView) {
        outImage->view = 0;
        vulkanImageViewCreate(context, format, outImage, viewAspectFlags);
    }
}

void vulkanImageViewCreate(
    VulkanContext* context,
    VkFormat format,
    VulkanImage* image,
    VkImageAspectFlags aspectFlags
) {
    VkImageViewCreateInfo viewCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewCreateInfo.image = image->handle;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format = format;
    viewCreateInfo.subresourceRange.aspectMask = aspectFlags;

    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(context->device.logicalDevice, &viewCreateInfo,
        context->allocator, &image->view))
}

void vulkanImageTransitionLayout(
    VulkanContext *context,
    VulkanCommandBuffer *commandBuffer,
    VulkanImage *image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout) {

    VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = context->device.graphicsQueueIndex;
    barrier.dstQueueFamilyIndex = context->device.graphicsQueueIndex;
    barrier.image = image->handle;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destStage;

    /** Don't care about the old layout - transition to optimal layout (for the underlying implementation). */
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {

        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        destStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        destStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        ENGINE_FATAL("Unsupported layout transition!")
        return;
    }

    vkCmdPipelineBarrier(
        commandBuffer->handle,
        sourceStage, destStage,
        0,
        0, 0,
        0, 0,
        1, &barrier);
}

void vulkanImageCopyFromBuffer(
    VulkanContext *context,
    VulkanImage *image,
    VkBuffer buffer,
    VulkanCommandBuffer *commandBuffer) {

    VkBufferImageCopy region;
    engineZeroMemory(&region, sizeof(VkBufferImageCopy));
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageExtent.width = image->width;
    region.imageExtent.height = image->height;
    region.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(
        commandBuffer->handle,
        buffer,
        image->handle,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);
}

void vulkanImageDestroy(VulkanContext* context, VulkanImage* image) {
    if (image->view) {
        vkDestroyImageView(context->device.logicalDevice, image->view, context->allocator);
        image->view = 0;
    }

    if (image->memory) {
        vkFreeMemory(context->device.logicalDevice, image->memory, context->allocator);
        image->memory = 0;
    }

    if (image->handle) {
        vkDestroyImage(context->device.logicalDevice, image->handle, context->allocator);
        image->handle = 0;
    }
}
