cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

set(PROJECT_NAME Editor)
project(${PROJECT_NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    src/main.c
)

target_link_libraries(${PROJECT_NAME} Engine)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    src/main.c
)
