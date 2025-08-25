#ifndef __ENGINE_VULKAN_PLATFORM_H__
#define __ENGINE_VULKAN_PLATFORM_H__

#include "../../defines.h"

struct PlatformState;
struct VulkanContext;

b8 platformCreateVulkanSurface(struct VulkanContext *context);

/**
 * Appends the names of required extensions for this platform to
 * the namesDynamicArray, which should be created and passed in.
 */
void platformGetRequiredExtensionNames(const char ***namesDynamicArray);

#endif
