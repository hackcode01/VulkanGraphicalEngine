#ifndef __VULKAN_PLATFORM_H__
#define __VULKAN_PLATFORM_H__

#include "../../defines.h"

/**
 * Appends the names of required extensions for this platform to
 * the namesDynamicArray, which should be created and passed in.
 */
void platformGetRequiredExtensionNames(const char*** namesDynamicArray);

#endif
