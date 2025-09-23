#include "vulkan_material_shader.h"

#include "../../../core/logger.h"
#include "../../../engine_memory/engine_memory.h"
#include "../../../engine_math/math_types.h"
#include "../../../engine_math/engine_math.h"

#include "../vulkan_shader_utils.h"
#include "../vulkan_pipeline.h"
#include "../vulkan_buffer.h"

#include "../../../systems/texture_system.h"

#define BUILTIN_SHADER_NAME_MATERIAL "BuiltinMaterialShader"

b8 vulkanMaterialShaderCreate(VulkanContext *context, VulkanMaterialShader *outShader) {
    /** Shader module init per stage. */
    char stageTypeStrs[MATERIAL_SHADER_STAGE_COUNT][5] = {"vert", "frag"};
    VkShaderStageFlagBits stageTypes[MATERIAL_SHADER_STAGE_COUNT] =
        {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};

    for (u32 i = 0; i < MATERIAL_SHADER_STAGE_COUNT; ++i) {
        if (!createShaderModule(context, BUILTIN_SHADER_NAME_MATERIAL, stageTypeStrs[i],
                                stageTypes[i], i, outShader->stages)) {
            ENGINE_ERROR("Unable to create %s shader module for '%s'.", stageTypeStrs[i],
                BUILTIN_SHADER_NAME_MATERIAL)

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

    /** Local/Object Descriptors. */
    VkDescriptorType descriptorTypes[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT] = {
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
    };
    VkDescriptorSetLayoutBinding bindings[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT];
    engineZeroMemory(&bindings, sizeof(VkDescriptorSetLayoutBinding) *
        VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT);
    for (u32 i = 0; i < VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT; ++i) {
        bindings[i].binding = i;
        bindings[i].descriptorCount = 1;
        bindings[i].descriptorType = descriptorTypes[i];
        bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutInfo.bindingCount = VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT;
    layoutInfo.pBindings = bindings;
    VK_CHECK(vkCreateDescriptorSetLayout(context->device.logicalDevice, &layoutInfo, 0,
        &outShader->objectDescriptorSetLayout))

    /** Local/Object descriptor pool: Used for object-specific items like diffuse colour. */
    VkDescriptorPoolSize objectPoolSizes[2];
    objectPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    objectPoolSizes[0].descriptorCount = VULKAN_MAX_MATERIAL_COUNT;

    objectPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    objectPoolSizes[1].descriptorCount = VULKAN_MATERIAL_SHADER_SAMPLER_COUNT *
                                         VULKAN_MAX_MATERIAL_COUNT;

    VkDescriptorPoolCreateInfo objectPoolInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    objectPoolInfo.poolSizeCount = 2;
    objectPoolInfo.pPoolSizes = objectPoolSizes;
    objectPoolInfo.maxSets = VULKAN_MAX_MATERIAL_COUNT;
    objectPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    /** Create object descriptor pool. */
    VK_CHECK(vkCreateDescriptorPool(context->device.logicalDevice, &objectPoolInfo,
        context->allocator, &outShader->objectDescriptorPool))

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

#define ATTRIBUTE_COUNT 2
    VkVertexInputAttributeDescription attributeDescriptions[ATTRIBUTE_COUNT];

    VkFormat formats[ATTRIBUTE_COUNT] = {
        VK_FORMAT_R32G32B32_SFLOAT,
        VK_FORMAT_R32G32_SFLOAT
    };
    u64 sizes[ATTRIBUTE_COUNT] = {
        sizeof(vec3),
        sizeof(vec2)
    };

    for (u32 i = 0; i < ATTRIBUTE_COUNT; ++i) {
        attributeDescriptions[i].binding = 0;
        attributeDescriptions[i].location = i;
        attributeDescriptions[i].format = formats[i];
        attributeDescriptions[i].offset = offset;
        offset += sizes[i];
    }

    /** Desciptor set layouts. */
    const i32 descriptorSetLayoutCount = 2;
    VkDescriptorSetLayout layouts[2] = {
        outShader->globalDescriptorSetLayout,
        outShader->objectDescriptorSetLayout
    };

    /**
     * Should match the number of shader->stages.
     */
    VkPipelineShaderStageCreateInfo stageCreateInfos[MATERIAL_SHADER_STAGE_COUNT];
    engineZeroMemory(stageCreateInfos, sizeof(stageCreateInfos));
    for (u32 i = 0; i < MATERIAL_SHADER_STAGE_COUNT; ++i) {
        stageCreateInfos[i].sType = outShader->stages[i].shaderStageCreateInfo.sType;
        stageCreateInfos[i] = outShader->stages[i].shaderStageCreateInfo;
    }

    if (!vulkanGraphicsPipelineCreate(
        context,
        &context->mainRenderpass,
        ATTRIBUTE_COUNT,
        attributeDescriptions,
        descriptorSetLayoutCount,
        layouts,
        MATERIAL_SHADER_STAGE_COUNT,
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

    if (!vulkanBufferCreate(context, sizeof(MaterialUniformObject) * VULKAN_MAX_MATERIAL_COUNT,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        true,
        &outShader->objectUniformBuffer)) {

        ENGINE_ERROR("Material instance buffer creation failed for shader.")
        return false;
    }

    return true;
}

void vulkanMaterialShaderDestroy(VulkanContext *context, struct VulkanMaterialShader *shader) {
    VkDevice logicalDevice = context->device.logicalDevice;

    vkDestroyDescriptorPool(logicalDevice, shader->objectDescriptorPool, context->allocator);
    vkDestroyDescriptorSetLayout(logicalDevice, shader->objectDescriptorSetLayout,
        context->allocator);

    /** Destroy uniform buffer. */
    vulkanBufferDestroy(context, &shader->globalUniformBuffer);
    vulkanBufferDestroy(context, &shader->objectUniformBuffer);

    /** Destroy pipeline. */
    vulkanPipelineDestroy(context, &shader->pipeline);

    /** Destroy global descriptor pool. */
    vkDestroyDescriptorPool(logicalDevice, shader->globalDescriptorPool,
        context->allocator);

    /** Destroy descriptor set layouts. */
    vkDestroyDescriptorSetLayout(logicalDevice, shader->globalDescriptorSetLayout,
        context->allocator);

    /** Destroy shader modules. */
    for (u32 i = 0; i < MATERIAL_SHADER_STAGE_COUNT; ++i) {
        vkDestroyShaderModule(context->device.logicalDevice, shader->stages[i].handle,
            context->allocator);
        shader->stages[i].handle = 0;
    }
}

void vulkanMaterialShaderUse(VulkanContext *context, struct VulkanMaterialShader *shader) {
    u32 imageIndex = context->imageIndex;
    vulkanPipelineBind(&context->graphicsCommandBuffers[imageIndex],
        VK_PIPELINE_BIND_POINT_GRAPHICS, &shader->pipeline);
}

void vulkanMaterialShaderUpdateGlobalState(VulkanContext *context,
    struct VulkanMaterialShader *shader, f32 deltaTime) {

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

void vulkanMaterialShaderUpdateObject(VulkanContext *context,
    struct VulkanMaterialShader *shader, GeometryRenderData data) {

    u32 imageIndex = context->imageIndex;
    VkCommandBuffer commandBuffer = context->graphicsCommandBuffers[imageIndex].handle;

    vkCmdPushConstants(commandBuffer, shader->pipeline.pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), &data.model);

    /** Obtain material data. */
    VulkanMaterialShaderInstanceState *objectState = &shader->instanceStates[data.material->internalId];
    VkDescriptorSet objectDescriptorSet = objectState->descriptorSets[imageIndex];

    /** If needs update. */
    VkWriteDescriptorSet descriptorWrites[VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT];
    engineZeroMemory(descriptorWrites, sizeof(VkWriteDescriptorSet) * VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT);
    u32 descriptorCount = 0;
    u32 descriptorIndex = 0;

    u32 range = sizeof(MaterialUniformObject);
    u64 offset = sizeof(MaterialUniformObject) * data.material->internalId;
    MaterialUniformObject obo;

    obo.diffuseColor = data.material->diffuseColour;

    vulkanBufferLoadData(context, &shader->objectUniformBuffer, offset, range, 0, &obo);

    /** Only do this if the descriptor has not yet been updated. */
    u32 *globalUBOGeneration = &objectState->descriptorStates[descriptorIndex]
        .generations[imageIndex];
    if (*globalUBOGeneration == INVALID_ID ||
        *globalUBOGeneration != data.material->generation) {

        VkDescriptorBufferInfo bufferInfo;
        bufferInfo.buffer = shader->objectUniformBuffer.handle;
        bufferInfo.offset = offset;
        bufferInfo.range = range;

        VkWriteDescriptorSet descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        descriptor.dstSet = objectDescriptorSet;
        descriptor.dstBinding = descriptorIndex;
        descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor.descriptorCount = 1;
        descriptor.pBufferInfo = &bufferInfo;

        descriptorWrites[descriptorCount] = descriptor;
        descriptorCount++;

        *globalUBOGeneration = data.material->generation;
    }
    descriptorIndex++;

    const u32 samplerCount = 1;
    VkDescriptorImageInfo imageInfos[1];
    for (u32 samplerIndex = 0; samplerIndex < samplerCount; ++samplerIndex) {
        TextureUse use = shader->samplerUses[samplerIndex];
        Texture *texture = 0;
        switch (use) {
            case TEXTURE_USE_MAP_DIFFUSE: {
                texture = data.material->diffuseMap.texture;
                break;
            }

            default: {
                ENGINE_FATAL("Unable to bind sampler to uknown use.")
                return;
            }
        }

        u32 *descriptorGeneration = &objectState->descriptorStates[descriptorIndex]
                                                  .generations[imageIndex];
        u32 *descriptorId = &objectState->descriptorStates[descriptorIndex].ids[imageIndex];

        if (texture->generation == INVALID_ID) {
            texture = textureSystemGetDefaultTexture();
            *descriptorGeneration = INVALID_ID;
        }

        if (texture && (*descriptorId != texture->id ||
            *descriptorGeneration != texture->generation ||
            *descriptorGeneration == INVALID_ID)) {

            VulkanTextureData *internalData = (VulkanTextureData*)texture->internalData;

            imageInfos[samplerIndex].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[samplerIndex].imageView = internalData->image.view;
            imageInfos[samplerIndex].sampler = internalData->sampler;

            VkWriteDescriptorSet descriptor = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            descriptor.dstSet = objectDescriptorSet;
            descriptor.dstBinding = descriptorIndex;
            descriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor.descriptorCount = 1;
            descriptor.pImageInfo = &imageInfos[samplerIndex];

            descriptorWrites[descriptorCount] = descriptor;
            descriptorCount++;

            if (texture->generation != INVALID_ID) {
                *descriptorGeneration = texture->generation;
                *descriptorId = texture->id;
            }
            descriptorIndex++;
        }
    }

    if (descriptorCount > 0) {
        vkUpdateDescriptorSets(context->device.logicalDevice, descriptorCount,
            descriptorWrites, 0, 0);
    }

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        shader->pipeline.pipelineLayout, 1, 1, &objectDescriptorSet, 0, 0);
}

b8 vulkanMaterialShaderAcquireResources(VulkanContext *context,
    struct VulkanMaterialShader *shader, Material *material) {

    material->internalId = shader->objectUniformBufferIndex;
    shader->objectUniformBufferIndex++;

    VulkanMaterialShaderInstanceState *objectState = &shader->instanceStates[material->internalId];
    for (u32 i = 0; i < VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT; ++i) {
        for (u32 j = 0; j < 3; ++j) {
            objectState->descriptorStates[i].generations[j] = INVALID_ID;
            objectState->descriptorStates[i].ids[j] = INVALID_ID;
        }
    }

    VkDescriptorSetLayout layouts[3] = {
        shader->objectDescriptorSetLayout,
        shader->objectDescriptorSetLayout,
        shader->objectDescriptorSetLayout
    };

    VkDescriptorSetAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocateInfo.descriptorPool = shader->objectDescriptorPool;
    allocateInfo.descriptorSetCount = 3;
    allocateInfo.pSetLayouts = layouts;

    VkResult result = vkAllocateDescriptorSets(context->device.logicalDevice, &allocateInfo, objectState->descriptorSets);
    if (result != VK_SUCCESS) {
        ENGINE_ERROR("Error allocating descriptor sets in shader!")
        return false;
    }

    return true;
}

void vulkanMaterialShaderReleaseResources(VulkanContext *context,
    struct VulkanMaterialShader *shader, Material *material) {

    VulkanMaterialShaderInstanceState *instanceState = &shader->instanceStates[material->internalId];

    const u32 descriptorSetCount = 3;
    VkResult result = vkFreeDescriptorSets(context->device.logicalDevice, shader->objectDescriptorPool, descriptorSetCount, instanceState->descriptorSets);
    if (result != VK_SUCCESS) {
        ENGINE_ERROR("Error freeing object shader descriptor sets!")
    }

    for (u32 i = 0; i < VULKAN_MATERIAL_SHADER_DESCRIPTOR_COUNT; ++i) {
        for (u32 j = 0; j < 3; ++j) {
            instanceState->descriptorStates[i].generations[j] = INVALID_ID;
            instanceState->descriptorStates[i].ids[j] = INVALID_ID;
        }
    }

    material->internalId = INVALID_ID;
}
