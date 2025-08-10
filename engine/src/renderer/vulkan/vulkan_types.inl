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

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;
} VulkanDevice;

typedef struct VulkanContext {
    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;

#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debugMessenger;
#endif

    VulkanDevice device;
} VulkanContext;

#endif
