cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

set(PROJECT_NAME VulkanGraphicalEngine)
project(${PROJECT_NAME})

add_subdirectory(engine)
add_subdirectory(editor)
add_subdirectory(tests)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY VS_STARTUP_PROJECT Editor
)
