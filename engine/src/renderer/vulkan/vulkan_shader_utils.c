#include "vulkan_shader_utils.h"

#include "../../engine_memory/engine_string.h"
#include "../../core/logger.h"
#include "../../engine_memory/engine_memory.h"

#include "../../platform/filesystem.h"

b8 createShaderModule(
    VulkanContext *context,
    const char *name,
    const char *typeStr,
    VkShaderStageFlagBits shaderStageFlag,
    u32 stageIndex,
    VulkanShaderStage *shaderStages) {

    /** Build file name. */
    char fileName[512];
    stringFormat(fileName, "./assets/shaders/%s.%s.spv", name, typeStr);

    engineZeroMemory(&shaderStages[stageIndex].createInfo, sizeof(VkShaderModuleCreateInfo));
    shaderStages[stageIndex].createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    /** Obtain file handle. */
    FileHandle handle;

    if (!filesystemOpen(fileName, FILE_MODE_READ, true, &handle)) {
        ENGINE_ERROR("Unable to read shader module: %s.", fileName);
        return false;
    }

    /** Read the entire file as binary. */
    u64 size = 0;
    u8* fileBuffer = 0;
    if (!filesystemReadAllBytes(&handle, &fileBuffer, &size)) {
        ENGINE_ERROR("Unable to binary read shader module: %s.", fileName);
        return false;
    }

    shaderStages[stageIndex].createInfo.codeSize = size;
    shaderStages[stageIndex].createInfo.pCode = (u32*)fileBuffer;

    /** Close the file. */
    filesystemClose(&handle);

    VK_CHECK(vkCreateShaderModule(
        context->device.logicalDevice,
        &shaderStages[stageIndex].createInfo,
        context->allocator,
        &shaderStages[stageIndex].handle)
    );

    // Shader stage info
    engineZeroMemory(&shaderStages[stageIndex].shaderStageCreateInfo, sizeof(VkPipelineShaderStageCreateInfo));
    shaderStages[stageIndex].shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[stageIndex].shaderStageCreateInfo.stage = shaderStageFlag;
    shaderStages[stageIndex].shaderStageCreateInfo.module = shaderStages[stageIndex].handle;
    shaderStages[stageIndex].shaderStageCreateInfo.pName = "main";

    if (fileBuffer) {
        engineFree(fileBuffer, sizeof(u8) * size, MEMORY_TAG_STRING);
        fileBuffer = 0;
    }

    return true;
}
