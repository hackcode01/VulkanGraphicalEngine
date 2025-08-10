#include "vulkan_device.h"

#include "../../core/logger.h"
#include "../../core/engine_string.h"
#include "../../core/engine_memory.h"

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

    return TRUE;
}

void vulkanDeviceDestroy(VulkanContext* context) {}

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
