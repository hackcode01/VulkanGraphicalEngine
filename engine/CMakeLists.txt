cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

set(PROJECT_NAME Engine)
project(${PROJECT_NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ENGINE_INCLUDES
    src/defines.h
    src/core/asserts.h
    src/core/logger.h
    src/platform/platform.h
)

set(ENGINE_SOURCES
    src/core/logger.c
    src/platform/platform_win32.c
)

set(ENGINE_ALL_SOURCES
    ${ENGINE_INCLUDES}
    ${ENGINE_SOURCES}
)

add_library(${PROJECT_NAME} STATIC
    ${ENGINE_ALL_SOURCES}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${ENGINE_ALL_SOURCES}
)
