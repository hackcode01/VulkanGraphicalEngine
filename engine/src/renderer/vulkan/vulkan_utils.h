#ifndef __VULKAN_UTILS_H__
#define __VULKAN_UTILS_H__

#include "vulkan_types.inl"

/**
 * Returns the string representation of result.
 * @param result The result to get the string for.
 * @param getExtented Indicates whether to also return an extented result.
 * @returns The error code and/or extented error message in string form.
 * Defaults to success for unknown result types.
 */
const char* vulkanResultString(VkResult result, b8 getExtented);

/**
 * Indicates if the passed is a success or an error as defined by the Vulkan spec.
 * @returns True if success, otherwise false. Defaults to true for unknown result types.
 */
b8 vulkanResultIsSuccess(VkResult result);

#endif
