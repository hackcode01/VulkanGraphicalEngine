#include "vulkan_object_shader.h"

#include "../../../core/logger.h"

#include "../vulkan_shader_utils.h"

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

    return true;
}

void vulkanObjectShaderDestroy(VulkanContext *context, struct VulkanObjectShader *shader) {}

void vulkanObjectShaderUse(VulkanContext *context, struct VulkanObjectShader *shader) {}
