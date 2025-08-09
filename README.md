# VulkanGraphicalEngine

A graphics engine based on Vulkan, developed with a view to the future.

# Build project on CMake

Run the following command to create the project configuration:
cmake -S . -B build

This will create configuration files for your platform, tested with clang and VC Visual Studio compilers on Windows.

Run the following command to build the project:
cmake --build build --config MinSizeRel

You can omit the --target flag, in which case the project will be built with the default configuration (usually Debug).

This will build your project and create the library file engine/engine.lib and the executable file editor/editor.exe

# Build project on files
Run next command in root directory of project.

On Linux:
./build_all.sh
./run_editor.sh

On Windows:
./build_all.bat
./run_editor.bat
