#include "vulkan_fence.h"

#include "../../core/logger.h"

void vulkanFenceCreate(
    VulkanContext* context,
    b8 createSignaled,
    VulkanFence* outFence) {

    /** Make sure to signal the fence if required. */
    outFence->isSignaled = createSignaled;
    VkFenceCreateInfo fenceCreateInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if (outFence->isSignaled) {
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    VK_CHECK(vkCreateFence(
        context->device.logicalDevice,
        &fenceCreateInfo,
        context->allocator,
        &outFence->handle
    ))
}

void vulkanFenceDestroy(VulkanContext* context, VulkanFence* fence) {
    if (fence->handle) {
        vkDestroyFence(
            context->device.logicalDevice,
            fence->handle,
            context->allocator
        );
        fence->handle = 0;
    }
    fence->handle = false;
}

b8 vulkanFenceWait(VulkanContext* context, VulkanFence* fence, u64 timeout_ns) {
    if (!fence->isSignaled) {
        VkResult result = vkWaitForFences(
            context->device.logicalDevice,
            1,
            &fence->handle,
            true,
            timeout_ns
        );

        switch (result) {
            case VK_SUCCESS: {
                fence->isSignaled = true;
                return true;
            }

            case VK_TIMEOUT: {
                ENGINE_WARNING("vulkanFenceWait - Time out")
                break;
            }

            case VK_ERROR_DEVICE_LOST: {
                ENGINE_ERROR("vulkanFenceWait - VK_ERROR_DEVICE_LOST.")
                break;
            }

            case VK_ERROR_OUT_OF_HOST_MEMORY: {
                ENGINE_ERROR("vulkanFenceWait - VK_ERROR_OUT_OF_HOST_MEMORY.")
                break;
            }

            case VK_ERROR_OUT_OF_DEVICE_MEMORY: {
                ENGINE_ERROR("vulkanFenceWait - VK_ERROR_OUT_OF_DEVICE_MEMORY.")
                break;
            }

            default: {
                ENGINE_ERROR("vulkanFenceWait - An unknown error has ocurred.")
                break;
            }
        }
    } else {
        /** If already signaled, don't wait. */
        return true;
    }

    return false;
}

void vulkanFenceReset(VulkanContext* context, VulkanFence* fence) {
    if (fence->isSignaled) {
        VK_CHECK(vkResetFences(context->device.logicalDevice, 1, &fence->handle))
        fence->isSignaled = false;
    }
}
