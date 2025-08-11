#include "vulkan_backend.h"
#include "vulkan_types.inl"
#include "vulkan_platform.h"

#include "../../core/logger.h"
#include "../../core/engine_string.h"

#include "../../containers/dynamic_array.h"

#include "../../platform/platform.h"

#include "vulkan_device.h"
#include "vulkan_swapchain.h"

/** Static Vulkan context. */
static VulkanContext context;

VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData
);

i32 findMemoryIndex(u32 typeFilter, u32 propertyFlags);

b8 vulkanRendererBackendInitialize(RendererBackend* backend, const char* applicationName,
    struct PlatformState* platformState) {
    /** Function pointers. */
    context.findMemoryIndex = findMemoryIndex;

    /** Custom allocator. */
    context.allocator = 0;

    /** Setup Vulkan instance. */
    VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.apiVersion = VK_API_VERSION_1_4;
    appInfo.pApplicationName = applicationName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Vulkan Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    createInfo.pApplicationInfo = &appInfo;
    
    /** Obtain a list of required extensions. */
    const char** requiredExtensions = dynamicArrayCreate(const char*);

    /** Generic surface extension. */
    dynamicArrayPush(requiredExtensions, &VK_KHR_SURFACE_EXTENSION_NAME)

    /** Platform-specific extension(s) */
    platformGetRequiredExtensionNames(&requiredExtensions);

#if defined(_DEBUG)
    /** Debug utilities. */
    dynamicArrayPush(requiredExtensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME)

    ENGINE_DEBUG("Required extensions:")
    u32 length = dynamicArrayLength(requiredExtensions);
    for (u32 i = 0; i < length; ++i) {
        ENGINE_DEBUG(requiredExtensions[i])
    }
#endif

    createInfo.enabledExtensionCount = dynamicArrayLength(requiredExtensions);
    createInfo.ppEnabledExtensionNames = requiredExtensions;

    /** Validation layers. */
    const char** requiredValidationLayerNames = 0;
    u32 requiredValidationLayerCount = 0;

/**
 * If validation should be done, get a list of the required validation layer names
 * and make sure they exist. Validation layers should only be enabled on non-release builds.
 */
#if defined(_DEBUG)
    ENGINE_INFO("Validation layers enabled. Enumerating...")

    /** The list of validation layers required. */
    requiredValidationLayerNames = dynamicArrayCreate(const char*);
    dynamicArrayPush(requiredValidationLayerNames, &"VK_LAYER_KHRONOS_validation")
    requiredValidationLayerCount = dynamicArrayLength(requiredValidationLayerNames);

    /** Obtain a list of available validation layers. */
    u32 availableLayerCount = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount, 0))
    VkLayerProperties* availableLayers = dynamicArrayReserve(VkLayerProperties,
                                                             availableLayerCount);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers))

    /** Vefify all required layers are available. */
    for (u32 i = 0; i < requiredValidationLayerCount; ++i) {
        ENGINE_INFO("Searching for layer: %s...", requiredValidationLayerNames[i])
        b8 found = FALSE;
        for (u32 j = 0; j < availableLayerCount; ++j) {
            if (stringsEqual(requiredValidationLayerNames[i], availableLayers[j].layerName)) {
                found = TRUE;
                ENGINE_INFO("Found.")
                break;
            }
        }

        if (!found) {
            ENGINE_FATAL("Required validation layer is missing: %s",
                requiredValidationLayerNames[i])
            return FALSE;
        }
    }

    ENGINE_INFO("All required validation layers are present.")
#endif

    createInfo.enabledLayerCount = requiredValidationLayerCount;
    createInfo.ppEnabledLayerNames = requiredValidationLayerNames;

    VK_CHECK(vkCreateInstance(&createInfo, context.allocator, &context.instance));
    ENGINE_INFO("Vulkan Instance created.");

#if defined(_DEBUG)
    ENGINE_DEBUG("Creating Vulkan debugger...");
    u32 logSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debugCreateInfo.messageSeverity = logSeverity;
    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugCreateInfo.pfnUserCallback = vkDebugCallback;

    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");
    ENGINE_ASSERT_MESSAGE(func, "Failed to create debug messenger!");
    VK_CHECK(func(context.instance, &debugCreateInfo, context.allocator, &context.debugMessenger));
    ENGINE_DEBUG("Vulkan debugger created.");
#endif

    /** Surface in Engine. */
    ENGINE_DEBUG("Creating Vulkan surface...")
    if (!platformCreateVulkanSurface(platformState, &context)) {
        ENGINE_ERROR("Failed to create platform surface!")
        return FALSE;
    }
    ENGINE_DEBUG("Vulkan surface created!")

    /**
     * Device creation.
     */
    if (!vulkanDeviceCreate(&context)) {
        ENGINE_ERROR("Failed to create device!")
        return FALSE;
    }

    /** Swapchain. */
    vulkanSwapchainCreate(
        &context,
        context.framebufferWidth,
        context.framebufferHeight,
        &context.swapchain
    );

    ENGINE_INFO("Vulkan renderer initialized successfully.")
    return TRUE;
}

void vulkanRendererBackendShutdown(RendererBackend* backend) {
    /** Destroy in the opposite order of creation. */

    /** Swapchain */
    vulkanSwapchainDestroy(&context, &context.swapchain);

    ENGINE_DEBUG("Destroying Vulkan device...")
    vulkanDeviceDestroy(&context);

    ENGINE_DEBUG("Destroying Vulkan surface...")
    if (context.surface) {
        vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
        context.surface = 0;
    }

    ENGINE_DEBUG("Destroying Vulkan debugger...")

    if (context.debugMessenger) {
        PFN_vkDestroyDebugUtilsMessengerEXT func =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance,
            "vkDestroyDebugUtilsMessengerEXT");
        func(context.instance, context.debugMessenger, context.allocator);
    }

    ENGINE_DEBUG("Destroying Vulkan instance...")
    vkDestroyInstance(context.instance, context.allocator);
}

void vulkanRendererBackendOnResize(RendererBackend* backend, u16 width, u16 height) {}

b8 vulkanRendererBackendBeginFrame(RendererBackend* backend, f32 deltaTime) {
    return TRUE;
}

b8 vulkanRendererBackendEndFrame(RendererBackend* backend, f32 deltaTime) {
    return TRUE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData
) {
    switch (messageSeverity) {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
            ENGINE_ERROR(callbackData->pMessage)
            break;
        }

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
            ENGINE_WARNING(callbackData->pMessage)
            break;
        }

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
            ENGINE_INFO(callbackData->pMessage)
            break;
        }

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
            ENGINE_TRACE(callbackData->pMessage)
            break;
        }
    }

    return VK_FALSE;
}

i32 findMemoryIndex(u32 typeFilter, u32 propertyFlags) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(context.device.physicalDevice, &memoryProperties);

    for (u32 i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        /** Check each memory type to see if its bit is set to 1. */
        if (typeFilter & (1 << i) &&
            (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) ==
                propertyFlags) {
            return i;
        }
    }

    ENGINE_WARNING("Unable to find suitable memory type!")
    return -1;
}
