#include "vulkan_object_shader.h"

#include "../../../core/logger.h"
#include "../../../engine_memory/engine_memory.h"
#include "../../../engine_math/math_types.h"

#include "../vulkan_shader_utils.h"
#include "../vulkan_pipeline.h"
#include "../vulkan_buffer.h"

#define BUILTIN_SHADER_NAME_OBJECT "BuiltinObjectShader"

b8 vulkanObjectShaderCreate(VulkanContext *context, VulkanObjectShader *outShader) {
    /** Shader module init per stage. */
    char stageTypeStrs[OBJECT_SHADER_STAGE_COUNT][10] = {"vert", "frag"};
        VkShaderStageFlagBits stageTypes[OBJECT_SHADER_STAGE_COUNT] =
            {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        if (!createShaderModule(context, BUILTIN_SHADER_NAME_OBJECT, stageTypeStrs[i],
                                stageTypes[i], i, outShader->stages)) {
            ENGINE_ERROR("Unable to create %s shader module for '%s'.", stageTypeStrs[i],
                BUILTIN_SHADER_NAME_OBJECT);

            return false;
        }
    }

    /** Global descriptors. */
    VkDescriptorSetLayoutBinding globalUBOLayoutBinding;
    globalUBOLayoutBinding.binding = 0;
    globalUBOLayoutBinding.descriptorCount = 1;
    globalUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    globalUBOLayoutBinding.pImmutableSamplers = 0;
    globalUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo globalLayoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    globalLayoutInfo.bindingCount = 1;
    globalLayoutInfo.pBindings = &globalUBOLayoutBinding;
    VK_CHECK(vkCreateDescriptorSetLayout(context->device.logicalDevice,
        &globalLayoutInfo, context->allocator, &outShader->globalDescriptorSetLayout))

    /** Global descriptor pool: Used for global items such as view/projection matrix. */
    VkDescriptorPoolSize globalPoolSize;
    globalPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    globalPoolSize.descriptorCount = context->swapchain.imageCount;

    VkDescriptorPoolCreateInfo globalPoolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    globalPoolInfo.poolSizeCount = 1;
    globalPoolInfo.pPoolSizes = &globalPoolSize;
    globalPoolInfo.maxSets = context->swapchain.imageCount;
    VK_CHECK(vkCreateDescriptorPool(context->device.logicalDevice, &globalPoolInfo,
        context->allocator, &outShader->globalDescriptorPool))

    /** Pipeline creation. */
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (f32)context->framebufferHeight;
    viewport.width = (f32)context->framebufferWidth;
    viewport.height = -(f32)context->framebufferHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    /** Scissor. */
    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context->framebufferWidth;
    scissor.extent.height = context->framebufferHeight;

    /** Attributes. */
    u32 offset = 0;
    const i32 attributeCount = 1;
    VkVertexInputAttributeDescription attribute_descriptions[1];

    /** Position. */
    VkFormat formats[1] = {VK_FORMAT_R32G32B32_SFLOAT};

    u64 sizes[1] = {sizeof(vec3)};

    for (u32 i = 0; i < attributeCount; ++i) {
        attribute_descriptions[i].binding = 0;
        attribute_descriptions[i].location = i;
        attribute_descriptions[i].format = formats[i];
        attribute_descriptions[i].offset = offset;
        offset += sizes[i];
    }

    /** Desciptor set layouts. */
    const i32 descriptorSetLayoutCount = 1;
    VkDescriptorSetLayout layouts[1] = {outShader->globalDescriptorSetLayout};

    /**
     * Should match the number of shader->stages.
     */
    VkPipelineShaderStageCreateInfo stageCreateInfos[OBJECT_SHADER_STAGE_COUNT];
    engineZeroMemory(stageCreateInfos, sizeof(stageCreateInfos));
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        stageCreateInfos[i].sType = outShader->stages[i].shaderStageCreateInfo.sType;
        stageCreateInfos[i] = outShader->stages[i].shaderStageCreateInfo;
    }

    if (!vulkanGraphicsPipelineCreate(
        context,
        &context->mainRenderpass,
        attributeCount,
        attribute_descriptions,
        descriptorSetLayoutCount,
        layouts,
        OBJECT_SHADER_STAGE_COUNT,
        stageCreateInfos,
        viewport,
        scissor,
        false,
        &outShader->pipeline)) {

        ENGINE_ERROR("Failed to load graphics pipeline for object shader.")
        return false;
    }

    if (!vulkanBufferCreate(context, sizeof(GlobalUniformObject),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        true,
        &outShader->globalUniformBuffer)) {

        ENGINE_ERROR("Vulkan buffer creation failed for object shader.")
        return false;
    }

    /** Allocate global descriptor sets. */
    VkDescriptorSetLayout globalLayouts[3] = {
        outShader->globalDescriptorSetLayout,
        outShader->globalDescriptorSetLayout,
        outShader->globalDescriptorSetLayout
    };

    VkDescriptorSetAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocateInfo.descriptorPool = outShader->globalDescriptorPool;
    allocateInfo.descriptorSetCount = 3;
    allocateInfo.pSetLayouts = globalLayouts;
    VK_CHECK(vkAllocateDescriptorSets(context->device.logicalDevice, &allocateInfo,
        outShader->globalDescriptorSets))

    return true;
}

void vulkanObjectShaderDestroy(VulkanContext *context, struct VulkanObjectShader *shader) {
    VkDevice logicalDevice = context->device.logicalDevice;

    /** Destroy uniform buffer. */
    vulkanBufferDestroy(context, &shader->globalUniformBuffer);

    /** Destroy pipeline. */
    vulkanPipelineDestroy(context, &shader->pipeline);

    /** Destroy global descriptor pool. */
    vkDestroyDescriptorPool(logicalDevice, shader->globalDescriptorPool,
        context->allocator);

    /** Destroy descriptor set layouts. */
    vkDestroyDescriptorSetLayout(logicalDevice, shader->globalDescriptorSetLayout,
        context->allocator);

    /** Destroy shader modules. */
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        vkDestroyShaderModule(context->device.logicalDevice, shader->stages[i].handle,
            context->allocator);
        shader->stages[i].handle = 0;
    }
}

void vulkanObjectShaderUse(VulkanContext *context, struct VulkanObjectShader *shader) {
    u32 imageIndex = context->imageIndex;
    vulkanPipelineBind(&context->graphicsCommandBuffers[imageIndex],
        VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
}

void vulkanObjectShaderUpdateGlobalState(VulkanContext *context, struct VulkanObjectShader *shader) {
    u32 imageIndex = context->imageIndex;
    VkCommandBuffer commandBuffer = context->graphicsCommandBuffers[imageIndex].handle;
    VkDescriptorSet globalDescriptor = shader->globalDescriptorSets[imageIndex];

    /** Bind the global descriptor set to be updated. */
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        shader->pipeline.pipelineLayout, 0, 1, &globalDescriptor, 0, 0);

    /** Configure the descriptors for the given index. */
    u32 range = sizeof(GlobalUniformObject);
    u64 offset = 0;

    /** Copy data to buffer. */
    vulkanBufferLoadData(context, &shader->globalUniformBuffer, offset, range, 0,
        &shader->globalUBO);

    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = shader->globalUniformBuffer.handle;
    bufferInfo.offset = offset;
    bufferInfo.range = range;

    /** Update descriptor sets. */
    VkWriteDescriptorSet descriptorWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descriptorWrite.dstSet = shader->globalDescriptorSets[imageIndex];
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(context->device.logicalDevice, 1, &descriptorWrite, 0, 0);
}
