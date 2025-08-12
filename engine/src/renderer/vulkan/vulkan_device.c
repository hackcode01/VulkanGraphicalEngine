#include "vulkan_device.h"

#include "../../core/logger.h"
#include "../../engine_memory/engine_string.h"
#include "../../engine_memory/engine_memory.h"

#include "../../containers/dynamic_array.h"

typedef struct VulkanPhysicalDeviceRequirements {
    b8 graphics;
    b8 present;
    b8 compute;
    b8 transfer;

    const char** deviceExtensionNames;

    b8 samplerAnisotropy;
    b8 discrete_gpu;
} VulkanPhysicalDeviceRequirements;

typedef struct VulkanPhysicalDeviceQueueFamilyInfo {
    u32 graphicsFamilyIndex;
    u32 presentFamilyIndex;
    u32 computeFamilyIndex;
    u32 transferFamilyIndex;
} VulkanPhysicalDeviceQueueFamilyInfo;

b8 selectPhysicalDevice(VulkanContext* context);
b8 physicalDeviceMeetsRequirements(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const VulkanPhysicalDeviceRequirements* requirements,
    VulkanPhysicalDeviceQueueFamilyInfo* outQueueFamilyInfo,
    VulkanSwapchainSupportInfo* outSwapchainSupport
);

b8 vulkanDeviceCreate(VulkanContext* context) {
    if (!selectPhysicalDevice(context)) {
        return FALSE;
    }

    ENGINE_INFO("Creating logical device...")
    /**
     * Do not create additional queues for shared indices.
     */
    b8 present_shares_graphics_queue = context->device.graphicsQueueIndex ==
                                       context->device.presentQueueIndex;
    b8 transfer_shares_graphics_queue = context->device.graphicsQueueIndex ==
                                        context->device.transferQueueIndex;
    u32 index_count = 1;

    if (!present_shares_graphics_queue) {
        index_count++;
    }

    if (!transfer_shares_graphics_queue) {
        index_count++;
    }

    u32 indices[index_count];
    u8 index = 0;
    indices[index++] = context->device.graphicsQueueIndex;

    if (!present_shares_graphics_queue) {
        indices[index++] = context->device.presentQueueIndex;
    }

    if (!transfer_shares_graphics_queue) {
        indices[index++] = context->device.transferQueueIndex;
    }

    VkDeviceQueueCreateInfo queue_create_infos[index_count];
    for (u32 i = 0; i < index_count; ++i) {
        queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex = indices[i];
        queue_create_infos[i].queueCount = 1;

        queue_create_infos[i].flags = 0;
        queue_create_infos[i].pNext = 0;
        f32 queue_priority = 1.0f;
        queue_create_infos[i].pQueuePriorities = &queue_priority;
    }

    /**
     * Request device features.
     * should be config driven
     */
    VkPhysicalDeviceFeatures device_features = {};
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo device_create_info = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    device_create_info.queueCreateInfoCount = index_count;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = 1;

    const char* extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    device_create_info.ppEnabledExtensionNames = &extension_names;

    /** Deprecated and ignored, so pass nothing. */
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = 0;

    /** Create the device. */
    VK_CHECK(vkCreateDevice(
        context->device.physicalDevice,
        &device_create_info,
        context->allocator,
        &context->device.logicalDevice)
    )

    ENGINE_INFO("Logical device created.")

    /** Get queues. */
    vkGetDeviceQueue(
        context->device.logicalDevice,
        context->device.graphicsQueueIndex,
        0,
        &context->device.graphicsQueue
    );

    vkGetDeviceQueue(
        context->device.logicalDevice,
        context->device.presentQueueIndex,
        0,
        &context->device.presentQueue
    );

    vkGetDeviceQueue(
        context->device.logicalDevice,
        context->device.transferQueueIndex,
        0,
        &context->device.transferQueue
    );
    ENGINE_INFO("Queues obtained.")

    /** Create command pool for graphics queue. */
    VkCommandPoolCreateInfo poolCreateInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    poolCreateInfo.queueFamilyIndex = context->device.graphicsQueueIndex;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_CHECK(vkCreateCommandPool(
        context->device.logicalDevice,
        &poolCreateInfo,
        context->allocator,
        &context->device.graphicsCommandPool
    ))
    ENGINE_INFO("Graphics command pool created.")

    return TRUE;
}

void vulkanDeviceDestroy(VulkanContext* context) {
    /** Unset queues */
    context->device.graphicsQueue = 0;
    context->device.presentQueue = 0;
    context->device.transferQueue = 0;

    ENGINE_INFO("Destroying command pools...")
    vkDestroyCommandPool(
        context->device.logicalDevice,
        context->device.graphicsCommandPool,
        context->allocator
    );

    /** Destroy logical device */
    ENGINE_INFO("Destroying logical device...");
    if (context->device.logicalDevice) {
        vkDestroyDevice(context->device.logicalDevice, context->allocator);
        context->device.logicalDevice = 0;
    }

    /** Physical devices are not destroyed. */
    ENGINE_INFO("Releasing physical device resources...");

    context->device.physicalDevice = 0;

    if (context->device.swapchainSupport.formats) {
        engineFree(
            context->device.swapchainSupport.formats,
            sizeof(VkSurfaceFormatKHR) * context->device.swapchainSupport.formatCount,
            MEMORY_TAG_RENDERER
        );

        context->device.swapchainSupport.formats = 0;
        context->device.swapchainSupport.formatCount = 0;
    }

    if (context->device.swapchainSupport.presentModes) {
        engineFree(
            context->device.swapchainSupport.presentModes,
            sizeof(VkPresentModeKHR) * context->device.swapchainSupport.presentModeCount,
            MEMORY_TAG_RENDERER
        );

        context->device.swapchainSupport.presentModes = 0;
        context->device.swapchainSupport.presentModeCount = 0;
    }

    engineZeroMemory(
        &context->device.swapchainSupport.capabilities,
        sizeof(context->device.swapchainSupport.capabilities)
    );

    context->device.graphicsQueueIndex = -1;
    context->device.presentQueueIndex = -1;
    context->device.transferQueueIndex = -1;
}

void vulkanDeviceQuerySwapchainSupport(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    VulkanSwapchainSupportInfo* outSupportInfo) {
    /** Surface capabilities. */
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physicalDevice,
        surface,
        &outSupportInfo->capabilities
    ))

    /** Surface formats. */
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice,
        surface,
        &outSupportInfo->formatCount,
        0
    ))

    if (outSupportInfo->formatCount != 0) {
        if (!outSupportInfo->formats) {
            outSupportInfo->formats = engineAllocate(sizeof(VkSurfaceFormatKHR) *
                outSupportInfo->formatCount, MEMORY_TAG_RENDERER);
        }

        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice,
            surface,
            &outSupportInfo->formatCount,
            outSupportInfo->formats
        ))
    }

    /** Present modes */
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice,
        surface,
        &outSupportInfo->presentModeCount,
        0
    ))

    if (outSupportInfo->presentModeCount != 0) {
        if (!outSupportInfo->presentModes) {
            outSupportInfo->presentModes = engineAllocate(sizeof(VkPresentModeKHR) *
                outSupportInfo->presentModeCount, MEMORY_TAG_RENDERER);
        }

        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice,
            surface,
            &outSupportInfo->presentModeCount,
            outSupportInfo->presentModes
        ))
    }
}

b8 vulkanDeviceDetectDepthFormat(VulkanDevice* device) {
    /** Format candidates. */
    const u64 candidateCount = 3ull;
    VkFormat candidates[3] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for (u64 i = 0; i < candidateCount; ++i) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(device->physicalDevice, candidates[i],
            &properties);

        if ((properties.linearTilingFeatures & flags) == flags) {
            device->depthFormat = candidates[i];
            return TRUE;
        } else if ((properties.optimalTilingFeatures & flags) == flags) {
            device->depthFormat = candidates[i];
            return TRUE;
        }
    }

    return FALSE;
}

b8 selectPhysicalDevice(VulkanContext* context) {
    u32 physicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physicalDeviceCount, 0))

    if (physicalDeviceCount == 0) {
        ENGINE_FATAL("No devices which support Vulkan were found.")
        return FALSE;
    }

    VkPhysicalDevice physicalDevices[physicalDeviceCount];
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physicalDeviceCount, physicalDevices))
    for (u32 i = 0; i < physicalDeviceCount; ++i) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(physicalDevices[i], &features);

        VkPhysicalDeviceMemoryProperties memory;
        vkGetPhysicalDeviceMemoryProperties(physicalDevices[i], &memory);

        /**
         * These requirements should probably be driven by engine
         * configuration.
         */
        VulkanPhysicalDeviceRequirements requirements = {};
        requirements.graphics = TRUE;
        requirements.present = TRUE;
        requirements.transfer = TRUE;
        requirements.samplerAnisotropy = TRUE;
        requirements.discrete_gpu = TRUE;
        requirements.deviceExtensionNames = dynamicArrayCreate(const char*);
        dynamicArrayPush(requirements.deviceExtensionNames, &VK_KHR_SWAPCHAIN_EXTENSION_NAME)

        VulkanPhysicalDeviceQueueFamilyInfo queue_info = {};
        b8 result = physicalDeviceMeetsRequirements(
            physicalDevices[i],
            context->surface,
            &properties,
            &features,
            &requirements,
            &queue_info,
            &context->device.swapchainSupport);

        if (result) {
            ENGINE_INFO("Selected device: '%s'.", properties.deviceName)
            /** GPU type, etc. */
            switch (properties.deviceType) {
                default:
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    ENGINE_INFO("GPU type is Unknown.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    ENGINE_INFO("GPU type is Integrated.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    ENGINE_INFO("GPU type is Descrete.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    ENGINE_INFO("GPU type is Virtual.");
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    ENGINE_INFO("GPU type is CPU.");
                    break;
            }

            ENGINE_INFO(
                "GPU Driver version: %d.%d.%d",
                VK_VERSION_MAJOR(properties.driverVersion),
                VK_VERSION_MINOR(properties.driverVersion),
                VK_VERSION_PATCH(properties.driverVersion)
            )

            /** Vulkan API version. */
            ENGINE_INFO(
                "Vulkan API version: %d.%d.%d",
                VK_VERSION_MAJOR(properties.apiVersion),
                VK_VERSION_MINOR(properties.apiVersion),
                VK_VERSION_PATCH(properties.apiVersion)
            )

            /** Memory information. */
            for (u32 j = 0; j < memory.memoryHeapCount; ++j) {
                f32 memory_size_gib = (((f32)memory.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);

                if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                    ENGINE_INFO("Local GPU memory: %.2f GiB", memory_size_gib);
                } else {
                    ENGINE_INFO("Shared System memory: %.2f GiB", memory_size_gib);
                }
            }

            context->device.physicalDevice = physicalDevices[i];
            context->device.graphicsQueueIndex = queue_info.graphicsFamilyIndex;
            context->device.presentQueueIndex = queue_info.presentFamilyIndex;
            context->device.transferQueueIndex = queue_info.transferFamilyIndex;

            /** Keep a copy of properties, features and memory info for later use. */
            context->device.properties = properties;
            context->device.features = features;
            context->device.memory = memory;
            break;
        }
    }

    /** Ensure a device was selected. */
    if (!context->device.physicalDevice) {
        ENGINE_ERROR("No physical devices were found which meet the requirements.");
        return FALSE;
    }

    ENGINE_INFO("Physical device selected.");
    return TRUE;
} 

b8 physicalDeviceMeetsRequirements(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const VulkanPhysicalDeviceRequirements* requirements,
    VulkanPhysicalDeviceQueueFamilyInfo* out_queue_info,
    VulkanSwapchainSupportInfo* out_swapchain_support) {
    /** Evalute device properties to determine if it meets the needs of our application. */
    out_queue_info->graphicsFamilyIndex = -1;
    out_queue_info->presentFamilyIndex = -1;
    out_queue_info->computeFamilyIndex = -1;
    out_queue_info->transferFamilyIndex = -1;

    /** Discrete GPU? */
    if (requirements->discrete_gpu) {
        if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            ENGINE_INFO("Device is not a discrete GPU, and one is required. Skipping.");
            return FALSE;
        }
    }

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 0);
    VkQueueFamilyProperties queue_families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

    /** Look at each queue and see what queues it supports */
    ENGINE_INFO("Graphics | Present | Compute | Transfer | Name");
    u8 min_transfer_score = 255;
    for (u32 i = 0; i < queue_family_count; ++i) {
        u8 current_transfer_score = 0;

        /** Graphics queue? */
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            out_queue_info->graphicsFamilyIndex = i;
            ++current_transfer_score;
        }

        /** Compute queue? */
        if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            out_queue_info->computeFamilyIndex = i;
            ++current_transfer_score;
        }

        /** Transfer queue? */
        if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            /**
             * Take the index if it is the current lowest. This increases the
             * liklihood that it is a dedicated transfer queue.
             */
            if (current_transfer_score <= min_transfer_score) {
                min_transfer_score = current_transfer_score;
                out_queue_info->transferFamilyIndex = i;
            }
        }

        // Present queue?
        VkBool32 supports_present = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supports_present))
        if (supports_present) {
            out_queue_info->presentFamilyIndex = i;
        }
    }

    /** Print out some info about the device */
    ENGINE_INFO("       %d |       %d |       %d |        %d | %s",
          out_queue_info->graphicsFamilyIndex != -1,
          out_queue_info->presentFamilyIndex != -1,
          out_queue_info->computeFamilyIndex != -1,
          out_queue_info->transferFamilyIndex != -1,
          properties->deviceName);

    if (
        (!requirements->graphics || (requirements->graphics && out_queue_info->graphicsFamilyIndex != -1)) &&
        (!requirements->present || (requirements->present && out_queue_info->presentFamilyIndex != -1)) &&
        (!requirements->compute || (requirements->compute && out_queue_info->computeFamilyIndex != -1)) &&
        (!requirements->transfer || (requirements->transfer && out_queue_info->transferFamilyIndex != -1))) {

        ENGINE_INFO("Device meets queue requirements.")
        ENGINE_TRACE("Graphics Family Index: %i", out_queue_info->graphicsFamilyIndex)
        ENGINE_TRACE("Present Family Index:  %i", out_queue_info->presentFamilyIndex)
        ENGINE_TRACE("Transfer Family Index: %i", out_queue_info->transferFamilyIndex)
        ENGINE_TRACE("Compute Family Index:  %i", out_queue_info->computeFamilyIndex)

        /** Query swapchain support. */
        vulkanDeviceQuerySwapchainSupport(
            device,
            surface,
            out_swapchain_support);

        if (out_swapchain_support->formatCount < 1 || out_swapchain_support->presentModeCount < 1) {
            if (out_swapchain_support->formats) {
                engineFree(out_swapchain_support->formats, sizeof(VkSurfaceFormatKHR) * out_swapchain_support->formatCount, MEMORY_TAG_RENDERER);
            }

            if (out_swapchain_support->presentModes) {
                engineFree(out_swapchain_support->presentModes, sizeof(VkPresentModeKHR) * out_swapchain_support->presentModeCount, MEMORY_TAG_RENDERER);
            }
            ENGINE_INFO("Required swapchain support not present, skipping device.");
            return FALSE;
        }

        /** Device extensions. */
        if (requirements->deviceExtensionNames) {
            u32 available_extension_count = 0;
            VkExtensionProperties* available_extensions = 0;
            VK_CHECK(vkEnumerateDeviceExtensionProperties(
                device,
                0,
                &available_extension_count,
                0));
            if (available_extension_count != 0) {
                available_extensions = engineAllocate(sizeof(VkExtensionProperties) * available_extension_count, MEMORY_TAG_RENDERER);
                VK_CHECK(vkEnumerateDeviceExtensionProperties(
                    device,
                    0,
                    &available_extension_count,
                    available_extensions));

                u32 required_extension_count = dynamicArrayLength(requirements->deviceExtensionNames);
                for (u32 i = 0; i < required_extension_count; ++i) {
                    b8 found = FALSE;
                    for (u32 j = 0; j < available_extension_count; ++j) {
                        if (stringsEqual(requirements->deviceExtensionNames[i], available_extensions[j].extensionName)) {
                            found = TRUE;
                            break;
                        }
                    }

                    if (!found) {
                        ENGINE_INFO("Required extension not found: '%s', skipping device.", requirements->deviceExtensionNames[i]);
                        engineFree(available_extensions, sizeof(VkExtensionProperties) * available_extension_count, MEMORY_TAG_RENDERER);
                        return FALSE;
                    }
                }
            }

            engineFree(available_extensions, sizeof(VkExtensionProperties) *
                available_extension_count, MEMORY_TAG_RENDERER);
        }

        /** Sampler anisotropy. */
        if (requirements->samplerAnisotropy && !features->samplerAnisotropy) {
            ENGINE_INFO("Device does not support samplerAnisotropy, skipping.");
            return FALSE;
        }

        // Device meets all requirements.
        return TRUE;
    }

    return FALSE;
}
