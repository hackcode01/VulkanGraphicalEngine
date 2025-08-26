cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

set(PROJECT_NAME Engine)
project(${PROJECT_NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

find_package(Vulkan REQUIRED COMPONENTS glslc)

set(INCLUDE_FILES
    src/defines.h
    src/game_types.h

    src/containers/dynamic_array.h

    src/core/application.h
    src/core/asserts.h
    src/core/clock.h
    src/core/event.h
    src/core/input.h
    src/core/logger.h

    src/engine_memory/engine_memory.h
    src/engine_memory/engine_string.h

    src/engine_memory/linear_allocator.h

    src/engine_math/engine_math.h
    src/engine_math/math_types.h

    src/platform/platform.h
    src/platform/filesystem.h

    src/renderer/renderer_backend.h
    src/renderer/renderer_frontend.h
    src/renderer/renderer_types.inl

    src/renderer/vulkan/vulkan_backend.h
    src/renderer/vulkan/vulkan_command_buffer.h
    src/renderer/vulkan/vulkan_device.h
    src/renderer/vulkan/vulkan_fence.h
    src/renderer/vulkan/vulkan_framebuffer.h
    src/renderer/vulkan/vulkan_image.h
    src/renderer/vulkan/vulkan_platform.h
    src/renderer/vulkan/vulkan_render_pass.h
    src/renderer/vulkan/vulkan_swapchain.h
    src/renderer/vulkan/vulkan_types.inl
    src/renderer/vulkan/vulkan_utils.h
    src/renderer/vulkan/vulkan_shader_utils.h
    src/renderer/vulkan/vulkan_pipeline.h

    src/renderer/vulkan/shaders/vulkan_object_shader.h
)

set(SOURCE_FILES
    src/containers/dynamic_array.c

    src/core/application.c
    src/core/clock.c
    src/core/event.c
    src/core/input.c
    src/core/logger.c

    src/engine_memory/engine_memory.c
    src/engine_memory/engine_string.c

    src/engine_memory/linear_allocator.c

    src/engine_math/engine_math.c

    src/platform/platform_win32.c
    src/platform/filesystem.c

    src/renderer/renderer_backend.c
    src/renderer/renderer_frontend.c

    src/renderer/vulkan/vulkan_backend.c
    src/renderer/vulkan/vulkan_command_buffer.c
    src/renderer/vulkan/vulkan_device.c
    src/renderer/vulkan/vulkan_fence.c
    src/renderer/vulkan/vulkan_framebuffer.c
    src/renderer/vulkan/vulkan_image.c
    src/renderer/vulkan/vulkan_render_pass.c
    src/renderer/vulkan/vulkan_swapchain.c
    src/renderer/vulkan/vulkan_utils.c
    src/renderer/vulkan/vulkan_shader_utils.c
    src/renderer/vulkan/vulkan_pipeline.c

    src/renderer/vulkan/shaders/vulkan_object_shader.c
)

add_library(${PROJECT_NAME} STATIC ${INCLUDE_FILES} ${SOURCE_FILES})
target_compile_options(${PROJECT_NAME} PRIVATE -g -shared -Wall)

file(GLOB_RECURSE GLSL_SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.comp"
)

foreach(GLSL IN LISTS GLSL_SOURCE_FILES)
    file(RELATIVE_PATH FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/shaders/" "${GLSL}")
    set(SPIRV "${CMAKE_SOURCE_DIR}/build/CompiledShaders/${FILE_NAME}.spv")

    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND Vulkan::glslc ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL})

    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    ShadersTarget ALL
    DEPENDS ${SPIRV_BINARY_FILES}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${INCLUDE_FILES} ${SOURCE_FILES}
)

target_compile_features(${PROJECT_NAME} PUBLIC c_std_17)

add_subdirectory(../external/glfw ${CMAKE_CURRENT_BINARY_DIR}/glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

add_subdirectory(../external/spdlog ${CMAKE_CURRENT_BINARY_DIR}/spdlog)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Vulkan::Vulkan
)

target_include_directories(${PROJECT_NAME} PRIVATE
    $<BUILD_INTERFACE:${glm_SOURCE_DIR}>
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    GLFW_INCLUDE_NONE
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        opengl32
        gdi32
    )
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(CORE_VIDEO_LIBRARY CoreVideo)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${CORE_VIDEO_LIBRARY}
    )
else()
    find_package(X11 REQUIRED)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        X11
        pthread
        dl
        ${CMAKE_THREAD_LIBS_INIT}
    )
endif()
