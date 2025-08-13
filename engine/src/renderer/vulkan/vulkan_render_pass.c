#include "vulkan_render_pass.h"

#include "../../engine_memory/engine_memory.h"

void vulkanRenderPassCreate(
    VulkanContext* context,
    VulkanRenderPass* outRenderpass,
    f32 coordinate_x, f32 coordinate_y, f32 width, f32 height,
    f32 red, f32 green, f32 blue, f32 alpha,
    f32 depth, u32 stencil) {
    /** Main subpass. */
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    /** Attachments: make this configurable. */
    u32 attachmentDescriptionCount = 2;
//#if defined(_MSC_VER) && defined(_WIN32)
//    VkAttachmentDescription attachmentDescriptions[2];
//#else
    VkAttachmentDescription attachmentDescriptions[attachmentDescriptionCount];
//#endif
    VkAttachmentDescription colorAttachment;
    colorAttachment.format = context->swapchain.imageFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    colorAttachment.flags = 0;

    attachmentDescriptions[0] = colorAttachment;

    VkAttachmentReference colorAttachmentReference;
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;

    /** Depth attachment, if there is one. */
    VkAttachmentDescription depthAttachment;
    depthAttachment.format = context->device.depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachmentDescriptions[1] = depthAttachment;

    /** Depth attachment reference. */
    VkAttachmentReference depth_attachment_reference;
    depth_attachment_reference.attachment = 1;
    depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    /** Depth stencil data. */
    subpass.pDepthStencilAttachment = &depth_attachment_reference;

    /** Input from a shader. */
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = 0;

    /** Attachments used for multisampling colour attachments. */
    subpass.pResolveAttachments = 0;

    /** Attachments not used in this subpass, but must be preserved for the next. */
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = 0;

    /** Render pass dependencies. Make this configurable. */
    VkSubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    /** Render pass create. */
    VkRenderPassCreateInfo renderPassCreateInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderPassCreateInfo.attachmentCount = attachmentDescriptionCount;
    renderPassCreateInfo.pAttachments = attachmentDescriptions;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;
    renderPassCreateInfo.pNext = 0;
    renderPassCreateInfo.flags = 0;

    VK_CHECK(vkCreateRenderPass(
        context->device.logicalDevice,
        &renderPassCreateInfo,
        context->allocator,
        &outRenderpass->handle))
}

void vulkanRenderPassDestroy(VulkanContext* context, VulkanRenderPass* renderpass) {
    if (renderpass && renderpass->handle) {
        vkDestroyRenderPass(context->device.logicalDevice,
            renderpass->handle, context->allocator);
        renderpass->handle = 0;
    }
}

void vulkanRenderPassBegin(
    VulkanCommandBuffer* commandBuffer,
    VulkanRenderPass* renderpass,
    VkFramebuffer frameBuffer) {

    VkRenderPassBeginInfo beginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    beginInfo.renderPass = renderpass->handle;
    beginInfo.framebuffer = frameBuffer;
    beginInfo.renderArea.offset.x = renderpass->coordinate_x;
    beginInfo.renderArea.offset.y = renderpass->coordinate_y;
    beginInfo.renderArea.extent.width = renderpass->width;
    beginInfo.renderArea.extent.height = renderpass->height;

    VkClearValue clearValues[2];
    engineZeroMemory(clearValues, sizeof(VkClearValue) * 2);
    clearValues[0].color.float32[0] = renderpass->red;
    clearValues[0].color.float32[1] = renderpass->green;
    clearValues[0].color.float32[2] = renderpass->blue;
    clearValues[0].color.float32[3] = renderpass->alpha;
    clearValues[1].depthStencil.depth = renderpass->depth;
    clearValues[1].depthStencil.stencil = renderpass->stencil;

    beginInfo.clearValueCount = 2;
    beginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffer->handle, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    commandBuffer->state = COMMAND_BUFFER_STATE_IN_RENDER_PASS;
}

void vulkanRenderPassEnd(VulkanCommandBuffer* commandBuffer,
    VulkanRenderPass* renderpass) {
    vkCmdEndRenderPass(commandBuffer->handle);
    commandBuffer->state = COMMAND_BUFFER_STATE_RECORDING;
}
