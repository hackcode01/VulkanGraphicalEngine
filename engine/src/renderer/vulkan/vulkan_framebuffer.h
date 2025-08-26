#ifndef __VULKAN_FRAMEBUFFER_H__
#define __VULKAN_FRAMEBUFFER_H__

#include "vulkan_types.inl"

void vulkanFramebufferCreate(
    VulkanContext* context,
    VulkanRenderpass* renderPass,
    u32 width, u32 height,
    u32 attachmentCount,
    VkImageView* attachments,
    VulkanFramebuffer* outFramebuffer
);

void vulkanFramebufferDestroy(VulkanContext* context, VulkanFramebuffer* framebuffer);

#endif
