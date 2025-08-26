#include "vulkan_object_shader.h"

#include "../../../core/logger.h"
#include "../../../engine_memory/engine_memory.h"
#include "../../../engine_math/math_types.h"

#include "../vulkan_shader_utils.h"
#include "../vulkan_pipeline.h"

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

    /** Descriptors. */

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
    VkFormat formats[1] = {
        VK_FORMAT_R32G32B32_SFLOAT
    };

    u64 sizes[1] = {
        sizeof(vec3)
    };

    for (u32 i = 0; i < attributeCount; ++i) {
        attribute_descriptions[i].binding = 0;
        attribute_descriptions[i].location = i;
        attribute_descriptions[i].format = formats[i];
        attribute_descriptions[i].offset = offset;
        offset += sizes[i];
    }

    /** Desciptor set layouts. */

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
            0,
            0,
            OBJECT_SHADER_STAGE_COUNT,
            stageCreateInfos,
            viewport,
            scissor,
            false,
            &outShader->pipeline)) {

        ENGINE_ERROR("Failed to load graphics pipeline for object shader.")
        return false;
    }

    return true;
}

void vulkanObjectShaderDestroy(VulkanContext *context, struct VulkanObjectShader *shader) {
    vulkanPipelineDestroy(context, &shader->pipeline);

    /** Destroy shader modules. */
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        vkDestroyShaderModule(context->device.logicalDevice, shader->stages[i].handle,
            context->allocator);
        shader->stages[i].handle = 0;
    }
}

void vulkanObjectShaderUse(VulkanContext *context, struct VulkanObjectShader *shader) {}
