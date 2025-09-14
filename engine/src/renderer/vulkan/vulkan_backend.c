#include "vulkan_backend.h"

#include "vulkan_types.inl"
#include "vulkan_platform.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_render_pass.h"
#include "vulkan_command_buffer.h"
#include "vulkan_framebuffer.h"
#include "vulkan_fence.h"
#include "vulkan_utils.h"
#include "vulkan_buffer.h"
#include "vulkan_image.h"

#include "../../core/logger.h"
#include "../../engine_memory/engine_string.h"
#include "../../engine_memory/engine_memory.h"
#include "../../core/application.h"

#include "../../containers/dynamic_array.h"

#include "../../engine_math/math_types.h"

#include "../../platform/platform.h"

/** Shaders. */
#include "shaders/vulkan_material_shader.h"

/** Static Vulkan context. */
static VulkanContext context;
static u32 cachedFramebufferWidth = 0;
static u32 cachedFramebufferHeight = 0;

VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData
);

i32 findMemoryIndex(u32 typeFilter, u32 propertyFlags);
b8 createBuffers(VulkanContext *context);

void createCommandBuffers(RendererBackend* backend);
void regenerateFramebuffers(RendererBackend* backend, VulkanSwapchain* swapchain,
    VulkanRenderpass* renderPass);
b8 recreateSwapchain(RendererBackend* backend);

void uploadDataRange(VulkanContext *context, VkCommandPool pool, VkFence fence,
    VkQueue queue, VulkanBuffer *buffer, u64 offset, u64 size, void *data) {
    /**
     * Create a host-visible staging buffer to upload to.
     * Mark it as the source of the transfer.
     */
    VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VulkanBuffer staging;
    vulkanBufferCreate(context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, flags, true, &staging);

    /** Load the data into the staging buffer. */
    vulkanBufferLoadData(context, &staging, 0, size, 0, data);

    /** Perform the copy from staging to the device local buffer. */
    vulkanBufferCopyTo(context, pool, fence, queue, staging.handle, 0,
        buffer->handle, offset, size);

    /** Clean up the staging buffer. */
    vulkanBufferDestroy(context, &staging);
}

b8 vulkanRendererBackendInitialize(RendererBackend *backend, const char *applicationName) {
    /** Function pointers. */
    context.findMemoryIndex = findMemoryIndex;

    /** Custom allocator. */
    context.allocator = 0;

    applicationGetFramebufferSize(&cachedFramebufferWidth, &cachedFramebufferHeight);
    context.framebufferWidth = (cachedFramebufferWidth != 0) ? cachedFramebufferWidth : 800;
    context.framebufferHeight = (cachedFramebufferHeight != 0) ? cachedFramebufferHeight : 600;
    cachedFramebufferWidth = 0;
    cachedFramebufferHeight = 0;

    /** Setup Vulkan instance. */
    VkApplicationInfo appInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.apiVersion = VK_API_VERSION_1_3;
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
        b8 found = false;
        for (u32 j = 0; j < availableLayerCount; ++j) {
            if (stringsEqual(requiredValidationLayerNames[i], availableLayers[j].layerName)) {
                found = true;
                ENGINE_INFO("Found.")
                break;
            }
        }

        if (!found) {
            ENGINE_FATAL("Required validation layer is missing: %s",
                requiredValidationLayerNames[i])
            return false;
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
    if (!platformCreateVulkanSurface(&context)) {
        ENGINE_ERROR("Failed to create platform surface!")
        return false;
    }
    ENGINE_DEBUG("Vulkan surface created!")

    /**
     * Device creation.
     */
    if (!vulkanDeviceCreate(&context)) {
        ENGINE_ERROR("Failed to create device!")
        return false;
    }

    /** Swapchain. */
    vulkanSwapchainCreate(
        &context,
        context.framebufferWidth,
        context.framebufferHeight,
        &context.swapchain
    );

    vulkanRenderPassCreate(
        &context,
        &context.mainRenderpass,
        0, 0, context.framebufferWidth, context.framebufferHeight,
        0.0f, 0.0f, 0.2f, 1.0f,
        1.0f,
        0
    );

    /** Swapchain framebuffers. */
    context.swapchain.framebuffers = dynamicArrayReserve(VulkanFramebuffer,
        context.swapchain.imageCount);
    regenerateFramebuffers(backend, &context.swapchain, &context.mainRenderpass);

    /** Create command buffers. */
    createCommandBuffers(backend);

    /** Create sync objects. */
    context.imageAvailableSemaphores = dynamicArrayReserve(VkSemaphore, context.swapchain.maxFramesInFlight);
    context.queueCompleteSemaphores = dynamicArrayReserve(VkSemaphore, context.swapchain.maxFramesInFlight);
    context.inFlightFences = dynamicArrayReserve(VulkanFence, context.swapchain.maxFramesInFlight);

    for (u8 i = 0; i < context.swapchain.maxFramesInFlight; ++i) {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(context.device.logicalDevice, &semaphoreCreateInfo,
            context.allocator, &context.imageAvailableSemaphores[i]);
        vkCreateSemaphore(context.device.logicalDevice, &semaphoreCreateInfo,
            context.allocator, &context.queueCompleteSemaphores[i]);

        /**
         * Create the fence in a signaled state, indicating that the first frame has already been "rendered".
         * This will prevent the application from waiting indefinitely for the first frame to render since it
         * cannot be rendered until a frame is "rendered" before it.
         */
        vulkanFenceCreate(&context, true, &context.inFlightFences[i]);
    }

    /**
     * In flight fences should not yet exist at this point, so clear the list. These are stored in pointers
     * because the initial state should be 0, and will be 0 when not in use. Acutal fences are not owned
     * by this list.
     */
    context.imagesInFlight = dynamicArrayReserve(VulkanFence, context.swapchain.imageCount);
    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        context.imagesInFlight[i] = 0;
    }

    /** Create builtin shaders. */
    if (!vulkanMaterialShaderCreate(&context, &context.materialShader)) {

        ENGINE_ERROR("Error loading builtin basicLightning shader.")
        return false;
    }

    createBuffers(&context);

    /** Temp test code. */
    const u32 vertexesCount = 4;
    vertex_3d vertexes[4];
    engineZeroMemory(vertexes, sizeof(vertex_3d) * vertexesCount);

    const f32 f = 10.0f;

    vertexes[0].position.x = -0.5 * f;
    vertexes[0].position.y = -0.5 * f;
    vertexes[0].textureCoord.x = 0.0f;
    vertexes[0].textureCoord.y = 0.0f;

    vertexes[1].position.x = 0.5 * f;
    vertexes[1].position.y = 0.5 * f;
    vertexes[1].textureCoord.x = 1.0f;
    vertexes[1].textureCoord.y = 1.0f;

    vertexes[2].position.x = -0.5 * f;
    vertexes[2].position.y = 0.5 * f;
    vertexes[2].textureCoord.x = 0.0f;
    vertexes[2].textureCoord.y = 1.0f;

    vertexes[3].position.x = 0.5 * f;
    vertexes[3].position.y = -0.5 * f;
    vertexes[3].textureCoord.x = 1.0f;
    vertexes[3].textureCoord.y = 0.0f;

    const u32 indexCount = 6;
    u32 indices[6] = {0, 1, 2, 0, 3, 1};

    uploadDataRange(&context, context.device.graphicsCommandPool, 0,
        context.device.graphicsQueue, &context.objectVertexBuffer, 0,
        sizeof(vertex_3d) * vertexesCount, vertexes);
    uploadDataRange(&context, context.device.graphicsCommandPool, 0,
        context.device.graphicsQueue, &context.objectIndexBuffer, 0,
        sizeof(u32) * indexCount, indices);

    u32 objectID = 0;
    if (!vulkanMaterialShaderAcquireResources(&context, &context.materialShader, &objectID)) {
        ENGINE_ERROR("Failed to acquire shader resources.")
        return false;
    }

    ENGINE_INFO("Vulkan renderer initialized successfully.")
    return true;
}

void vulkanRendererBackendShutdown(RendererBackend* backend) {
    vkDeviceWaitIdle(context.device.logicalDevice);

    /** Destroy in the opposite order of creation. */

    /** Destroy buffers. */
    vulkanBufferDestroy(&context, &context.objectVertexBuffer);
    vulkanBufferDestroy(&context, &context.objectIndexBuffer);

    vulkanMaterialShaderDestroy(&context, &context.materialShader);

    /** Sync objects. */
    for (u8 i = 0; i < context.swapchain.maxFramesInFlight; ++i) {
        if (context.imageAvailableSemaphores[i]) {
            vkDestroySemaphore(
                context.device.logicalDevice,
                context.imageAvailableSemaphores[i],
                context.allocator);
            context.imageAvailableSemaphores[i] = 0;
        }

        if (context.queueCompleteSemaphores[i]) {
            vkDestroySemaphore(
                context.device.logicalDevice,
                context.queueCompleteSemaphores[i],
                context.allocator);
            context.queueCompleteSemaphores[i] = 0;
        }

        vulkanFenceDestroy(&context, &context.inFlightFences[i]);
    }

    dynamicArrayDestroy(context.imageAvailableSemaphores);
    context.imageAvailableSemaphores = 0;

    dynamicArrayDestroy(context.queueCompleteSemaphores);
    context.queueCompleteSemaphores = 0;

    dynamicArrayDestroy(context.inFlightFences);
    context.inFlightFences = 0;

    dynamicArrayDestroy(context.imagesInFlight);
    context.imagesInFlight = 0;

    /** Command buffers. */
    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        if (context.graphicsCommandBuffers[i].handle) {
            vulkanCommandBufferFree(
                &context,
                context.device.graphicsCommandPool,
                &context.graphicsCommandBuffers[i]
            );
            context.graphicsCommandBuffers[i].handle = 0;
        }
    }

    dynamicArrayDestroy(context.graphicsCommandBuffers);
    context.graphicsCommandBuffers = 0;

    /** Destroy framebuffers */
    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        vulkanFramebufferDestroy(&context, &context.swapchain.framebuffers[i]);
    }

    /** Render pass. */
    vulkanRenderPassDestroy(&context, &context.mainRenderpass);

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

void vulkanRendererBackendOnResize(RendererBackend* backend, u16 width, u16 height) {
    /** Update the "framebuffer size generation", a counter which indicates when the
     * framebuffer size has been updated.
    */
    cachedFramebufferWidth = width;
    cachedFramebufferHeight = height;
    context.framebufferSizeGeneration++;

    ENGINE_INFO("Vulkan renderer backend->resized: w/h/gen: %i/%i/%llu",
        width, height, context.framebufferSizeGeneration);
}

b8 vulkanRendererBackendBeginFrame(RendererBackend* backend, f32 deltaTime) {
    context.frameDeltaTime = deltaTime;
    VulkanDevice* device = &context.device;

    /** Check if recreating swap chain and boot out. */
    if (context.recreatingSwapchain) {
        VkResult result = vkDeviceWaitIdle(device->logicalDevice);

        if (!vulkanResultIsSuccess(result)) {
            ENGINE_ERROR("vulkanRendererBackendBeginFrame vkDeviceWaitIdle (1) "
                "failed: '%s'", vulkanResultString(result, true));
            return false;
        }

        ENGINE_INFO("Recreating swapchain, booting.");
        return false;
    }

    /**
     * Check if the framebuffer has been resized. If so,
     * a new swapchain must be created.
     */
    if (context.framebufferSizeGeneration != context.framebufferSizeLastGeneration) {
        VkResult result = vkDeviceWaitIdle(device->logicalDevice);

        if (!vulkanResultIsSuccess(result)) {
            ENGINE_ERROR("vulkanRendererBackendBeginFrame vkDeviceWaitIdle (2) "
                "failed: '%s'", vulkanResultString(result, true));
            return false;
        }

        /**
         * If the swapchain recreation failed (because, for example,
         * the window was minimized), boot out before unsetting the flag.
         */
        if (!recreateSwapchain(backend)) {
            return false;
        }

        ENGINE_INFO("Resized, booting.");
        return false;
    }

    /**
     * Wait for the execution of the current frame to complete.
     * The fence being free will allow this one to move on.
     */
    if (!vulkanFenceWait(
        &context,
        &context.inFlightFences[context.currentFrame],
        ENGINE_UINT64_MAX)) {
    
        ENGINE_WARNING("In-flight fence wait failure!")
        return false;
    }

    /**
     * Acquire the next image from the swap chain.
     * Pass along the semaphore that should signaled when this completes.
     * This same semaphore will later be waited on by the queue submission to
     * ensure this image is available.
     */
    if (!vulkanSwapchainAcquireNextImageIndex(
        &context,
        &context.swapchain,
        ENGINE_UINT64_MAX,
        context.imageAvailableSemaphores[context.currentFrame],
        0,
        &context.imageIndex)) {

        return false;
    }

    /** Begin recording commands. */
    VulkanCommandBuffer* commandBuffer = &context.graphicsCommandBuffers[context.imageIndex];
    vulkanCommandBufferReset(commandBuffer);
    vulkanCommandBufferBegin(commandBuffer, false, false, false);

    /** Dynamic state. */
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (f32)context.framebufferHeight;
    viewport.width = (f32)context.framebufferWidth;
    viewport.height = -(f32)context.framebufferHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    /** Scissor. */
    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = context.framebufferWidth;
    scissor.extent.height = context.framebufferHeight;

    vkCmdSetViewport(commandBuffer->handle, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer->handle, 0, 1, &scissor);

    context.mainRenderpass.width = context.framebufferWidth;
    context.mainRenderpass.height = context.framebufferHeight;

    /** Begin the render pass. */
    vulkanRenderPassBegin(
        commandBuffer,
        &context.mainRenderpass,
        context.swapchain.framebuffers[context.imageIndex].handle
    );

    return true;
}

void vulkanRendererUpdateGlobalState(mat4 projection, mat4 view, vec3 viewPosition, vec4 ambientColour, i32 mode) {
    vulkanMaterialShaderUse(&context, &context.materialShader);

    context.materialShader.globalUBO.projection = projection;
    context.materialShader.globalUBO.view = view;

    /** Other UBO properties. */
    vulkanMaterialShaderUpdateGlobalState(&context, &context.materialShader,
        context.frameDeltaTime);
}

b8 vulkanRendererBackendEndFrame(RendererBackend* backend, f32 deltaTime) {
    VulkanCommandBuffer* command_buffer = &context.graphicsCommandBuffers[context.imageIndex];

    /** End renderpass. */
    vulkanRenderPassEnd(command_buffer, &context.mainRenderpass);

    vulkanCommandBufferEnd(command_buffer);

    /**
     * Make sure the previous frame is not using this image
     * (i.e. its fence is being waited on).
     */
    if (context.imagesInFlight[context.imageIndex] != VK_NULL_HANDLE) {
        vulkanFenceWait(
            &context,
            context.imagesInFlight[context.imageIndex],
            ENGINE_UINT64_MAX);
    }

    /** Mark the image fence as in-use by this frame. */
    context.imagesInFlight[context.imageIndex] = &context.inFlightFences[context.currentFrame];

    /** Reset the fence for use on the next frame. */
    vulkanFenceReset(&context, &context.inFlightFences[context.currentFrame]);

    /**
     * Submit the queue and wait for the operation to complete.
     * Begin queue submission.
     */
    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

    /** Command buffer(s) to be executed. */
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer->handle;

    /** The semaphore(s) to be signaled when the queue is complete. */
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &context.queueCompleteSemaphores[context.currentFrame];

    /** Wait semaphore ensures that the operation cannot begin until the image is available. */
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &context.imageAvailableSemaphores[context.currentFrame];

    /**
     * Each semaphore waits on the corresponding pipeline stage to complete. 1:1 ratio.
     * VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT prevents subsequent colour attachment
     * writes from executing until the semaphore signals (i.e. one frame is presented at a time).
     */
    VkPipelineStageFlags flags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.pWaitDstStageMask = flags;

    VkResult result = vkQueueSubmit(
        context.device.graphicsQueue,
        1,
        &submit_info,
        context.inFlightFences[context.currentFrame].handle);
    if (result != VK_SUCCESS) {
        ENGINE_ERROR("vkQueueSubmit failed with result: %s",
            vulkanResultString(result, true));
        return false;
    }

    vulkanCommandBufferUpdateSubmitted(command_buffer);

    /** Give the image back to the swapchain. */
    vulkanSwapchainPresent(
        &context,
        &context.swapchain,
        context.device.graphicsQueue,
        context.device.presentQueue,
        context.queueCompleteSemaphores[context.currentFrame],
        context.imageIndex);

    return true;
}

void vulkanBackendUpdateObject(GeometryRenderData data) {
    VulkanCommandBuffer* commandBuffer = &context.graphicsCommandBuffers[context.imageIndex];

    vulkanMaterialShaderUpdateObject(&context, &context.materialShader, data);

    vulkanMaterialShaderUse(&context, &context.materialShader);

    /** Bind vertex buffer at offset. */
    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(commandBuffer->handle, 0, 1, &context.objectVertexBuffer.handle, (VkDeviceSize*)offsets);

    /** Bind index buffer at offset. */
    vkCmdBindIndexBuffer(commandBuffer->handle, context.objectIndexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);

    /** Issue the draw. */
    vkCmdDrawIndexed(commandBuffer->handle, 6, 1, 0, 0, 0);
}

VKAPI_ATTR VkBool32 VKAPI_CALL vkDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData) {
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

void createCommandBuffers(RendererBackend* backend) {
    if (!context.graphicsCommandBuffers) {
        context.graphicsCommandBuffers = dynamicArrayReserve(VulkanCommandBuffer,
            context.swapchain.imageCount);

        for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
            engineZeroMemory(&context.graphicsCommandBuffers[i], sizeof(VulkanCommandBuffer));
        }
    }

    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        if (context.graphicsCommandBuffers[i].handle) {
            vulkanCommandBufferFree(
                &context,
                context.device.graphicsCommandPool,
                &context.graphicsCommandBuffers[i]);
        }

        engineZeroMemory(&context.graphicsCommandBuffers[i], sizeof(VulkanCommandBuffer));

        vulkanCommandBufferAllocate(
            &context,
            context.device.graphicsCommandPool,
            true,
            &context.graphicsCommandBuffers[i]);
    }

    ENGINE_DEBUG("Vulkan command buffers created.")
}

void regenerateFramebuffers(RendererBackend* backend, VulkanSwapchain* swapchain,
    VulkanRenderpass* renderPass) {
    for (u32 i = 0; i < swapchain->imageCount; ++i) {
        /** Make this dynamic based on the currently configured attachments */
        u32 attachmentCount = 2;
        VkImageView attachments[] = {
            swapchain->views[i],
            swapchain->depthAttachment.view
        };

        vulkanFramebufferCreate(
            &context,
            renderPass,
            context.framebufferWidth,
            context.framebufferHeight,
            attachmentCount,
            attachments,
            &context.swapchain.framebuffers[i]
        );
    }
}

b8 recreateSwapchain(RendererBackend* backend) {
    /** If already being recreated, do not try again. */
    if (context.recreatingSwapchain) {
        ENGINE_DEBUG("recreate_swapchain called when already recreating. Booting.");
        return false;
    }

    /** Detect if the window is too small to be drawn to. */
    if (context.framebufferWidth == 0 || context.framebufferHeight == 0) {
        ENGINE_DEBUG("recreate_swapchain called when window is < 1 in a dimension. Booting.");
        return false;
    }

    /** Mark as recreating if the dimensions are valid. */
    context.recreatingSwapchain = true;

    /** Wait for any operations to complete. */
    vkDeviceWaitIdle(context.device.logicalDevice);

    /** Clear these out just in case. */
    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        context.imagesInFlight[i] = 0;
    }

    /** Requery support. */
    vulkanDeviceQuerySwapchainSupport(
        context.device.physicalDevice,
        context.surface,
        &context.device.swapchainSupport);
    vulkanDeviceDetectDepthFormat(&context.device);

    vulkanSwapchainRecreate(
        &context,
        cachedFramebufferWidth,
        cachedFramebufferHeight,
        &context.swapchain);

    /** Sync the framebuffer size with the cached sizes. */
    context.framebufferWidth = cachedFramebufferWidth;
    context.framebufferHeight = cachedFramebufferHeight;
    context.mainRenderpass.width = context.framebufferWidth;
    context.mainRenderpass.height = context.framebufferHeight;
    cachedFramebufferWidth = 0;
    cachedFramebufferHeight = 0;

    /** Update framebuffer size generation. */
    context.framebufferSizeLastGeneration = context.framebufferSizeGeneration;

    /** cleanup swapchain. */
    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        vulkanCommandBufferFree(&context, context.device.graphicsCommandPool,
            &context.graphicsCommandBuffers[i]);
    }

    /** Framebuffers. */
    for (u32 i = 0; i < context.swapchain.imageCount; ++i) {
        vulkanFramebufferDestroy(&context, &context.swapchain.framebuffers[i]);
    }

    context.mainRenderpass.coordinate_x = 0;
    context.mainRenderpass.coordinate_y = 0;
    context.mainRenderpass.width = context.framebufferWidth;
    context.mainRenderpass.height = context.framebufferHeight;

    regenerateFramebuffers(backend, &context.swapchain, &context.mainRenderpass);

    createCommandBuffers(backend);

    /** Clear the recreating flag. */
    context.recreatingSwapchain = false;

    return true;
}

b8 createBuffers(VulkanContext *context) {
    VkMemoryPropertyFlagBits memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    const u64 vertexBufferSize = sizeof(vertex_3d) * 1024 * 1024;

    if (!vulkanBufferCreate(
        context,
        vertexBufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        memoryPropertyFlags,
        true,
        &context->objectVertexBuffer)) {

        ENGINE_ERROR("Error creating vertex buffer.")
        return false;
    }

    context->geometryVertexOffset = 0;

    const u64 indexBufferSize = sizeof(u32) * 1024 * 1024;

    if (!vulkanBufferCreate(
        context, indexBufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, memoryPropertyFlags, true,
        &context->objectIndexBuffer)) {
        ENGINE_ERROR("Error creating vertex buffer.")
        return false;
    }

    context->geometryIndexOffset = 0;

    return true;
}

void vulkanRendererCreateTexture(const char *name,
    i32 width, i32 height, i32 channelCount, const u8 *pixels,
    b8 hasTransparency, Texture *outTexture) {

    outTexture->width = width;
    outTexture->height = height;
    outTexture->channelCount = channelCount;
    outTexture->generation = INVALID_ID;

    outTexture->internalData = (VulkanTextureData*)engineAllocate(sizeof(VulkanTextureData), MEMORY_TAG_TEXTURE);
    VulkanTextureData *data = (VulkanTextureData*)outTexture->internalData;
    VkDeviceSize imageSize = width * height * channelCount;

    VkFormat imageFormat = VK_FORMAT_R8G8B8A8_UNORM;

    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    VulkanBuffer staging;
    vulkanBufferCreate(&context, imageSize, usage, memoryPropertyFlags, true, &staging);

    vulkanBufferLoadData(&context, &staging, 0, imageSize, 0, pixels);

    vulkanImageCreate(&context, VK_IMAGE_TYPE_2D, width, height, imageFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_COLOR_BIT,
        &data->image);

    VulkanCommandBuffer tempBuffer;
    VkCommandPool pool = context.device.graphicsCommandPool;
    VkQueue queue = context.device.graphicsQueue;
    vulkanCommandBufferAllocateAndBeginSingleUse(&context, pool, &tempBuffer);

    vulkanImageTransitionLayout(
        &context, &tempBuffer, &data->image, imageFormat, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vulkanImageCopyFromBuffer(&context, &data->image, staging.handle, &tempBuffer);

    /** Transition from optimal for data reciept to shader-read-only optimal layout. */
    vulkanImageTransitionLayout(&context, &tempBuffer, &data->image, imageFormat,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vulkanCommandBufferEndSingleUse(&context, pool, &tempBuffer, queue);

    vulkanBufferDestroy(&context, &staging);

    VkSamplerCreateInfo samplerInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    VkResult result = vkCreateSampler(context.device.logicalDevice, &samplerInfo,
        context.allocator, &data->sampler);

    if (!vulkanResultIsSuccess(VK_SUCCESS)) {
        ENGINE_ERROR("Error creating texture sampler: %s", vulkanResultString(result, true))
        return;
    }

    outTexture->hasTransparency = hasTransparency;
    outTexture->generation++;
}

void vulkanRendererDestroyTexture(struct Texture *texture) {
    vkDeviceWaitIdle(context.device.logicalDevice);

    VulkanTextureData *data = (VulkanTextureData*)texture->internalData;
    if (data) {
        vulkanImageDestroy(&context, &data->image);
        engineZeroMemory(&data->image, sizeof(VulkanImage));
        vkDestroySampler(context.device.logicalDevice, data->sampler, context.allocator);
        data->sampler = 0;
        engineFree(texture->internalData, sizeof(VulkanTextureData), MEMORY_TAG_TEXTURE);
    }

    engineZeroMemory(texture, sizeof(struct Texture));
}
