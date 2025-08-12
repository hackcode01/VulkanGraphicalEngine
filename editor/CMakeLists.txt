cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

set(PROJECT_NAME Editor)
project(${PROJECT_NAME})

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(INCLUDE_FILES
    src/game.h
)

set(SOURCE_FILES
    src/game.c
)

add_executable(${PROJECT_NAME} ${INCLUDE_FILES} ${SOURCE_FILES} src/main.c)

target_link_libraries(${PROJECT_NAME} Engine spdlog)

set_target_properties(${PROJECT_NAME}
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/bin/
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${INCLUDE_FILES} ${SOURCE_FILES} src/main.c
)
