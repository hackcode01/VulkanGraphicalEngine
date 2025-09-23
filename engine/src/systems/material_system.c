#include "material_system.h"

#include "../core/logger.h"
#include "../engine_memory/engine_string.h"
#include "../containers/hashtable.h"
#include "../engine_math/engine_math.h"
#include "../renderer/renderer_frontend.h"

#include "./texture_system.h"

#include "../platform/filesystem.h"

typedef struct MaterialSystemState {
    MaterialSystemConfig config;
    Material defaultMaterial;
    Material *registeredMaterials;
    Hashtable registeredMaterialTable;
} MaterialSystemState;

typedef struct MaterialReference {
    u64 referenceCount;
    u32 handle;
    b8 autoRelease;
} MaterialReference;

static MaterialSystemState *statePtr = 0;

b8 createDefaultMaterial(MaterialSystemState *state);
b8 loadMaterial(MaterialConfig config, Material *material);
void destroyMaterial(Material *material);
b8 loadConfigurationFile(const char *path, MaterialConfig *outConfig);

b8 materialSystemInitialize(u64* memoryRequirement, void* state, MaterialSystemConfig config) {
    if (config.maxMaterialCount == 0) {
        ENGINE_FATAL("MaterialSystemInitialize - config.maxMaterialCount must be > 0.")
        return false;
    }

    /**
     * Block of memory will contain state structure, then block for array,
     * then block for hashtable.
     */
    u64 structRequirement = sizeof(MaterialSystemState);
    u64 arrayRequirement = sizeof(Material) * config.maxMaterialCount;
    u64 hashtableRequirement = sizeof(MaterialReference) * config.maxMaterialCount;
    *memoryRequirement = structRequirement + arrayRequirement + hashtableRequirement;

    if (!state) {
        return true;
    }

    statePtr = state;
    statePtr->config = config;

    /** The array is after the state. Already allocated, so just set the pointer. */
    void *arrayBlock = state + structRequirement;
    statePtr->registeredMaterials = arrayBlock;

    /** Hashtable block is after array. */
    void *hashtableBlock = arrayBlock + arrayRequirement;

    /** Create a hashtable for material lookups. */
    hashtableCreate(sizeof(MaterialReference), config.maxMaterialCount, hashtableBlock,
                    false, &statePtr->registeredMaterialTable);

    /** Fill the hashtable with invalid references to use as a default. */
    MaterialReference invalidRef;
    invalidRef.autoRelease = false;
    invalidRef.handle = INVALID_ID;
    invalidRef.referenceCount = 0;
    hashtableFill(&statePtr->registeredMaterialTable, &invalidRef);

    u32 count = statePtr->config.maxMaterialCount;
    for (u32 i = 0; i < count; ++i) {
        statePtr->registeredMaterials[i].id = INVALID_ID;
        statePtr->registeredMaterials[i].generation = INVALID_ID;
        statePtr->registeredMaterials[i].internalId = INVALID_ID;
    }

    if (!createDefaultMaterial(statePtr)) {
        ENGINE_FATAL("Failed to create default material. Application cannot continue.")
        return false;
    }

    return true;
}

void materialSystemShutdown(void *state) {
    MaterialSystemState *materialSystemState = (MaterialSystemState*)state;
    if (materialSystemState) {
        u32 count = materialSystemState->config.maxMaterialCount;
        for (u32 i = 0; i < count; ++i) {
            if (materialSystemState->registeredMaterials[i].id != INVALID_ID) {
                destroyMaterial(&materialSystemState->registeredMaterials[i]);
            }
        }

        destroyMaterial(&materialSystemState->defaultMaterial);
    }

    statePtr = 0;
}

Material *materialSystemAcquire(const char *name) {
    MaterialConfig config;

    char *formatStr = "assets/materials/%s.%s";
    char fullFilePath[512];

    stringFormat(fullFilePath, formatStr, name, "kmt");
    if (!loadConfigurationFile(fullFilePath, &config)) {
        ENGINE_ERROR("Failed to load material file: '%s'. Null pointer will be returned.",
            fullFilePath)
        return 0;
    }

    return materialSystemAcquireFromConfig(config);
}

Material *materialSystemAcquireFromConfig(MaterialConfig config) {
    if (stringsEquali(config.name, DEFAULT_MATERIAL_NAME)) {
        return &statePtr->defaultMaterial;
    }

    MaterialReference ref;
    if (statePtr && hashtableGet(&statePtr->registeredMaterialTable, config.name, &ref)) {
        if (ref.referenceCount == 0) {
            ref.autoRelease = config.autoRelease;
        }
        ref.referenceCount++;
        if (ref.handle == INVALID_ID) {
            u32 count = statePtr->config.maxMaterialCount;
            Material *material = 0;
            for (u32 i = 0; i < count; ++i) {
                if (statePtr->registeredMaterials[i].id == INVALID_ID) {
                    ref.handle = i;
                    material = &statePtr->registeredMaterials[i];
                    break;
                }
            }

            if (!material || ref.handle == INVALID_ID) {
                ENGINE_FATAL("MaterialSystemAcquire - "
                    "Material system cannot hold anymore materials. "
                    "Adjust configuration to allow more.")
                return 0;
            }

            if (!loadMaterial(config, material)) {
                ENGINE_ERROR("Failed to load material '%s'.", config.name)
                return 0;
            }

            if (material->generation == INVALID_ID) {
                material->generation = 0;
            } else {
                material->generation++;
            }

            material->id = ref.handle;
            ENGINE_TRACE("Material '%s' does not yet exist. Created, and refCount is now %i",
                config.name, ref.referenceCount)
        } else {
            ENGINE_TRACE("Material '%s' already exists, refCount increased to %i",
                config.name, ref.referenceCount)
        }

        hashtableSet(&statePtr->registeredMaterialTable, config.name, &ref);
        return &statePtr->registeredMaterials[ref.handle];
    }

    ENGINE_ERROR("material_system_acquire_from_config failed to acquire material '%s'. "
        "Null pointer will be returned.", config.name)
    return 0;
}

void materialSystemRelease(const char *name) {
    if (stringsEqualI(name, DEFAULT_MATERIAL_NAME)) {
        return;
    }

    MaterialReference ref;
    if (statePtr && hashtableGet(&statePtr->registeredMaterialTable, name, &ref)) {
        if (ref.referenceCount == 0) {
            ENGINE_WARNING("Tried to release non-existent material: '%s'", name)
            return;
        }

        ref.referenceCount--;
        if (ref.referenceCount == 0 && ref.autoRelease) {
            Material *m = &statePtr->registeredMaterials[ref.handle];

            destroyMaterial(m);

            ref.handle = INVALID_ID;
            ref.autoRelease = false;
            ENGINE_TRACE("Released material '%s'., "
                "Material unloaded because reference count=0 and auto_release=true.",
                name)
        } else {
            ENGINE_TRACE("Released material '%s', "
                "now has a reference count of '%i' (auto_release=%s).",
                name, ref.referenceCount, ref.autoRelease ? "true" : "false")
        }

        hashtableSet(&statePtr->registeredMaterialTable, name, &ref);
    } else {
        ENGINE_ERROR("material_system_release failed to release material '%s'.", name)
    }
}

b8 createDefaultMaterial(MaterialSystemState *state) {
    engineZeroMemory(&state->defaultMaterial, sizeof(Material));
    state->defaultMaterial.id = INVALID_ID;
    state->defaultMaterial.generation = INVALID_ID;
    stringNCopy(state->defaultMaterial.name, DEFAULT_MATERIAL_NAME, MATERIAL_NAME_MAX_LENGTH);
    state->defaultMaterial.diffuseColour = vec4_one();
    state->defaultMaterial.diffuseMap.use = TEXTURE_USE_MAP_DIFFUSE;
    state->defaultMaterial.diffuseMap.texture = textureSystemGetDefaultTexture();

    if (!rendererCreateMaterial(&state->defaultMaterial)) {
        ENGINE_FATAL("Failed to acquire renderer resources for default texture. "
            "Application cannot continue.")
        return false;
    }

    return true;
}

b8 loadMaterial(MaterialConfig config, Material *material) {

}

void destroyMaterial(Material *material) {

}

b8 loadConfigurationFile(const char *path, MaterialConfig *outConfig) {

}
