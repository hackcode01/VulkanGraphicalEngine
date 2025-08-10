#ifndef __VULKAN_TYPES_INL__
#define __VULKAN_TYPES_INL__

#include "../../defines.h"

#include <vulkan/vulkan.h>

typedef struct VulkanContext {
    VkInstance instance;
    VkAllocationCallbacks* allocator;
} VulkanContext;

#endif
