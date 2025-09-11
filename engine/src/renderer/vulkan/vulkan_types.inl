#ifndef __VULKAN_TYPES_INL__
#define __VULKAN_TYPES_INL__

#include "../../defines.h"
#include "../../core/asserts.h"

#include "../renderer_types.inl"

#include <vulkan/vulkan.h>

/**
 * Checks the given expression's return value against VK_SUCCESS.
 */
#define VK_CHECK(expression) {              \
    ENGINE_ASSERT(expression == VK_SUCCESS) \
}

typedef struct VulkanBuffer {
    u64 totalSize;
    VkBuffer handle;
    VkBufferUsageFlagBits usage;
    b8 isLocked;
    VkDeviceMemory memory;
    i32 memoryIndex;
    u32 memoryPropertyFlags;
} VulkanBuffer;

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

    VkCommandPool graphicsCommandPool;

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

typedef enum VulkanRenderpassState {
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITTED,
    NOT_ALLOCATED
} VulkanRenderpassState;

typedef struct VulkanRenderpass {
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

    VulkanRenderpassState state;
} VulkanRenderpass;

typedef struct VulkanFramebuffer {
    VkFramebuffer handle;
    u32 attachmentCount;
    VkImageView* attachments;
    VulkanRenderpass* renderPass;
} VulkanFramebuffer;

typedef struct VulkanSwapchain {
    VkSurfaceFormatKHR imageFormat;
    u8 maxFramesInFlight;
    VkSwapchainKHR handle;
    u32 imageCount;
    VkImage* images;
    VkImageView* views;

    VulkanImage depthAttachment;

    /** Framebuffers used for on-screen rendering. */
    VulkanFramebuffer* framebuffers;
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

typedef struct VulkanFence {
    VkFence handle;
    b8 isSignaled;
} VulkanFence;

typedef struct VulkanShaderStage {
    VkShaderModuleCreateInfo createInfo;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
} VulkanShaderStage;

typedef struct VulkanPipeline {
    VkPipeline handle;
    VkPipelineLayout pipelineLayout;
} VulkanPipeline;

#define OBJECT_SHADER_STAGE_COUNT 2

typedef struct VulkanDescriptorState {
    u32 generations[3];
} VulkanDescriptorState;

#define VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT 2
typedef struct VulkanObjectShaderObjectState {
    VkDescriptorSet descriptorSets[3];

    VulkanDescriptorState descriptorStates[VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT];
} VulkanObjectShaderObjectState;

#define VULKAN_OBJECT_MAX_OBJECT_COUNT 1024

typedef struct VulkanObjectShader {
    /** Vertex and fragment shaders. */
    VulkanShaderStage stages[OBJECT_SHADER_STAGE_COUNT];

    VkDescriptorPool globalDescriptorPool;
    VkDescriptorSetLayout globalDescriptorSetLayout;

    /** One descriptor set per frame - max 3 for triple-buffering. */
    VkDescriptorSet globalDescriptorSets[3];

    /** Global uniform object. */
    GlobalUniformObject globalUBO;

    /** Global uniform buffer. */
    VulkanBuffer globalUniformBuffer;

    VkDescriptorPool objectDescriptorPool;
    VkDescriptorSetLayout objectDescriptorSetLayout;

    VulkanBuffer objectUniformBuffer;

    u32 objectUniformBufferIndex;

    VulkanObjectShaderObjectState objectStates[VULKAN_OBJECT_MAX_OBJECT_COUNT];

    /** Pointers to default textures. */
    Texture *defaultDiffuse;

    VulkanPipeline pipeline;
} VulkanObjectShader;

typedef struct VulkanContext {
    f32 frameDeltaTime;

    /** The framebuffer's current width. */
    u32 framebufferWidth;

    /** The framebuffer's current height. */
    u32 framebufferHeight;

    /**
     * Current generation of framebuffer size. If it does not match
     * framebufferSizeLastGeneration, a new one should be generated.
    */
   u64 framebufferSizeGeneration;

   /**
    * The generation of the framebuffer when it was last created.
    * Set to framebufferSizeGeneration when updated.
    */
   u64 framebufferSizeLastGeneration;

    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;

    VkDebugUtilsMessengerEXT debugMessenger;

    VulkanDevice device;

    VulkanSwapchain swapchain;
    VulkanRenderpass mainRenderpass;

    VulkanBuffer objectVertexBuffer;
    VulkanBuffer objectIndexBuffer;

    /** Dynamic array. */
    VulkanCommandBuffer* graphicsCommandBuffers;

    /** Dynamic array. */
    VkSemaphore* imageAvailableSemaphores;

    /** Dynamic array. */
    VkSemaphore* queueCompleteSemaphores;

    u32 inFlightFenceCount;
    VulkanFence* inFlightFences;

    /** Holds pointers to fences which exist and are owned elsewhere. */
    VulkanFence** imagesInFlight;

    u32 imageIndex;
    u32 currentFrame;

    b8 recreatingSwapchain;

    VulkanObjectShader objectShader;

    u64 geometryVertexOffset;
    u64 geometryIndexOffset;

    i32 (*findMemoryIndex)(u32 typeFilter, u32 propertyFlags);

} VulkanContext;

typedef struct VulkanTextureData {
    VulkanImage image;
    VkSampler sampler;
} VulkanTextureData;

#endif
