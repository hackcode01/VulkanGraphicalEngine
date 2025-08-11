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

typedef struct VulkanSwapchainSupportInfo {
    VkSurfaceCapabilitiesKHR capabilities;
    u32 formatCount;
    VkSurfaceFormatKHR* formats;
    u32 presentModeCount;
    VkPresentModeKHR* presentModes;
} VulkanSwapchainSupportInfo;

typedef struct VulkanDevice {
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    VulkanSwapchainSupportInfo swapchainSupport;

    i32 graphicsQueueIndex;
    i32 presentQueueIndex;
    i32 transferQueueIndex;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;

    VkFormat depthFormat;
} VulkanDevice;

typedef struct VulkanImage {
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    u32 width;
    u32 height;
} VulkanImage;

typedef enum VulkanRenderPassState {
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITTED,
    NOT_ALLOCATED
} VulkanRenderPassState;

typedef struct VulkanRenderPass {
    VkRenderPass handle;
    f32 coordinate_x;
    f32 coordinate_y;
    f32 width;
    f32 height;

    f32 red;
    f32 green;
    f32 blue;
    f32 alpha;

    f32 depth;
    u32 stencil;

    VulkanRenderPassState state;
} VulkanRenderPass;

typedef struct VulkanSwapchain {
    VkSurfaceFormatKHR imageFormat;
    u8 maxFramesInFlight;
    VkSwapchainKHR handle;
    u32 imageCount;
    VkImage* images;
    VkImageView* views;

    VulkanImage depthAttachment;
} VulkanSwapchain;

typedef enum VulkanCommandBufferState {
    COMMAND_BUFFER_STATE_READY,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_IN_RENDER_PASS,
    COMMAND_BUFFER_STATE_RECORDING_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED,
    COMMAND_BUFFER_STATE_NOT_ALLOCATED
} VulkanCommandBufferState;

typedef struct VulkanCommandBuffer {
    VkCommandBuffer handle;

    /** Command buffer state. */
    VulkanCommandBufferState state;
} VulkanCommandBuffer;

typedef struct VulkanContext {
    /** The framebuffer's current width. */
    u32 framebufferWidth;

    /** The framebuffer's current height. */
    u32 framebufferHeight;

    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;

#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debugMessenger;
#endif

    VulkanDevice device;

    VulkanSwapchain swapchain;
    VulkanRenderPass mainRenderpass;

    u32 imageIndex;
    u32 currentFrame;

    b8 recreatingSwapchain;

    i32 (*findMemoryIndex)(u32 typeFilter, u32 propertyFlags);
} VulkanContext;

#endif
