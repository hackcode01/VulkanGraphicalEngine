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

typedef struct VulkanContext {
    VkInstance instance;
    VkAllocationCallbacks* allocator;

#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debugMessenger;
#endif
} VulkanContext;

#endif
