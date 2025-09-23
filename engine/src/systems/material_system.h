#ifndef __ENGINE_MATERIAL_SYSTEM_H__
#define __ENGINE_MATERIAL_SYSTEM_H__

#include "../defines.h"

#include "../resources/resource_types.h"

#define DEFAULT_MATERIAL_NAME "default"

typedef struct MaterialSystemConfig {
    u32 maxMaterialCount;
} MaterialSystemConfig;

typedef struct MaterialConfig {
    char name[MATERIAL_NAME_MAX_LENGTH];
    b8 autoRelease;
    vec4 diffuseColour;
    char diffuseMapName[TEXTURE_NAME_MAX_LENGTH];
} MaterialConfig;

b8 materialSystemInitialize(u64 *memoryRequirement, void *state, MaterialSystemConfig config);
void materialSystemShutdown(void *state);

Material *materialSystemAcquire(const char *name);
Material *materialSystemAcquireFromConfig(MaterialConfig config);
void materialSystemRelease(const char *name);

#endif
