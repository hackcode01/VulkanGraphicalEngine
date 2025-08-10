#include "vulkan_backend.h"
#include "vulkan_types.inl"

#include "../../core/logger.h"

/** Static Vulkan context. */
static VulkanContext context;

b8 vulkanRendererBackendInitialize(RendererBackend* backend, const char* applicationName,
    struct PlatformState* platformState) {
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
    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = NULL;
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = NULL;

    VkResult result = vkCreateInstance(&createInfo, context.allocator, &context.instance);

    if (result != VK_SUCCESS) {
        ENGINE_ERROR("vkCreateInstance failed with result: %u", result)
        return FALSE;
    }

    ENGINE_INFO("Vulkan renderer initialized successfully.")
    return TRUE;
}

void vulkanRendererBackendShutdown(RendererBackend* backend) {}

void vulkanRendererBackendOnResize(RendererBackend* backend, u16 width, u16 height) {}

b8 vulkanRendererBackendBeginFrame(RendererBackend* backend, f32 deltaTime) {
    return TRUE;
}

b8 vulkanRendererBackendEndFrame(RendererBackend* backend, f32 deltaTime) {
    return TRUE;
}
