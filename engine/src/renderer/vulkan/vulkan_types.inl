#ifndef __VULKAN_TYPES_INL__
#define __VULKAN_TYPES_INL__

#include "../../defines.h"
#include "../../core/asserts.h"

#include <vulkan/vulkan.h>

/**
 * Checks the given expression's return value against VK_SUCCESS.
 */
#define VK_CHECK(expression) {              \
    ENGINE_ASSERT(expression == VK_SUCCESS) \
}

typedef struct VulkanSwapchainSupportInfo {
    VkSurfaceCapabilitiesKHR capabilities;
    u32 formatCount;
    VkSurfaceFormatKHR* formats;
    u32 presentModeCount;
    VkPresentModeKHR* presentModes;
} VulkanSwapchainSupportInfo;

typedef struct VulkanDevice {
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    VulkanSwapchainSupportInfo swapchainSupport;

    i32 graphicsQueueIndex;
    i32 presentQueueIndex;
    i32 transferQueueIndex;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;

    VkFormat depthFormat;
} VulkanDevice;

typedef struct VulkanImage {
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    u32 width;
    u32 height;
} VulkanImage;

typedef struct VulkanSwapchain {
    VkSurfaceFormatKHR imageFormat;
    u8 maxFramesInFlight;
    VkSwapchainKHR handle;
    u32 imageCount;
    VkImage* images;
    VkImageView* views;

    VulkanImage depthAttachment;
} VulkanSwapchain;

typedef struct VulkanContext {
    /** The framebuffer's current width. */
    u32 framebufferWidth;

    /** The framebuffer's current height. */
    u32 framebufferHeight;

    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;

#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debugMessenger;
#endif

    VulkanDevice device;

    VulkanSwapchain swapchain;
    u32 imageIndex;
    u32 currentFrame;

    b8 recreatingSwapchain;

    i32 (*findMemoryIndex)(u32 typeFilter, u32 propertyFlags);
} VulkanContext;

#endif
