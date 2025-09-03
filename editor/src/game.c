#include "game.h"

#include "../../engine/src/core/logger.h"
#include "../../engine/src/engine_memory/engine_memory.h"

#include "../../engine/src/core/input.h"

#include "../../engine/src/engine_math/engine_math.h"

/** HACK: This should not be available outside the engine. */
#include "../../engine/src/renderer/renderer_frontend.h"

void recalculateViewMatrix(GameState *state) {
    if (state->cameraViewDirty) {
        mat4 rotation = mat4_euler_xyz(state->cameraEuler.x, state->cameraEuler.y,
            state->cameraEuler.z);
        mat4 translation = mat4_translation(state->cameraPosition);

        state->view = mat4_mul(rotation, translation);
        state->view = mat4_inverse(state->view);

        state->cameraViewDirty = false;
    }
}

void cameraYaw(GameState *state, f32 amount) {
    state->cameraEuler.y += amount;
    state->cameraViewDirty = true;
}

void cameraPitch(GameState *state, f32 amount) {
    state->cameraEuler.x += amount;

    /** Clamp to avoid Gimball lock. */
    f32 limit = deg_to_rad(89.0f);
    state->cameraEuler.x = ENGINE_CLAMP(state->cameraEuler.x, -limit, limit)

    state->cameraViewDirty = true;
}

b8 gameInitialize(Game *gameInstance) {
    ENGINE_DEBUG("gameInitialize() called!")

    GameState *state = (GameState*)gameInstance->state;

    state->cameraPosition = (vec3){0, 0, 30.0f};
    state->cameraEuler = vec3_zero();

    state->view = mat4_translation(state->cameraPosition);
    state->view = mat4_inverse(state->view);
    state->cameraViewDirty = true;

    return true;
}

b8 gameUpdate(Game *gameInstance, f32 deltaTime) {
    static u64 allocationCount = 0;
    u64 prevAllocationCount = allocationCount;
    allocationCount = getMemoryAllocationCount();

    if (inputIsKeyUp('M') && inputWasKeyDown('M')) {
        ENGINE_DEBUG("Allocations: %llu (%llu this frame)",
            allocationCount, allocationCount - prevAllocationCount)
    }

    GameState *state = (GameState*)gameInstance->state;

    /** HACK: temp hack to move camera around. */
    if (inputIsKeyDown('A') || inputIsKeyDown(KEY_LEFT)) {
        cameraYaw(state, 1.0f * deltaTime);
    }

    if (inputIsKeyDown('D') || inputIsKeyDown(KEY_RIGHT)) {
        cameraYaw(state, -1.0f * deltaTime);
    }

    if (inputIsKeyDown(KEY_UP)) {
        cameraPitch(state, 1.0f * deltaTime);
    }

    if (inputIsKeyDown(KEY_DOWN)) {
        cameraPitch(state, -1.0f * deltaTime);
    }

    f32 tempMoveSpeed = 50.0f;
    vec3 velocity = vec3_zero();

    if (inputIsKeyDown('W')) {
        vec3 forward = mat4_forward(state->view);
        velocity = vec3_add(velocity, forward);
    }

    if (inputIsKeyDown('S')) {
        vec3 backward = mat4_backward(state->view);
        velocity = vec3_add(velocity, backward);
    }

    if (inputIsKeyDown('Q')) {
        vec3 left = mat4_left(state->view);
        velocity = vec3_add(velocity, left);
    }

    if (inputIsKeyDown('E')) {
        vec3 right = mat4_right(state->view);
        velocity = vec3_add(velocity, right);
    }

    if (inputIsKeyDown(KEY_SPACE)) {
        velocity.y += 1.0f;
    }

    if (inputIsKeyDown('X')) {
        velocity.y -= 1.0f;
    }

    vec3 z = vec3_zero();
    if (!vec3_compare(z, velocity, 0.0002f)) {
        /** Be sure to normalize the velocity before applying speed. */
        vec3_normalize(&velocity);
        state->cameraPosition.x += velocity.x * tempMoveSpeed * deltaTime;
        state->cameraPosition.y += velocity.y * tempMoveSpeed * deltaTime;
        state->cameraPosition.z += velocity.z * tempMoveSpeed * deltaTime;
        state->cameraViewDirty = true;
    }

    recalculateViewMatrix(state);

    /** HACK: This should not be available outside the engine. */
    rendererSetView(state->view);

    return true;
}

b8 gameRender(Game *gameInstance, f32 deltaTime) {
    return true;
}

void gameOnResize(Game *gameInstance, u32 width, u32 height) {}
