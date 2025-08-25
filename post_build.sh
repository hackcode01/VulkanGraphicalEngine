#!/bin/bash

# Run from root directory!
mkdir -p bin/assets
mkdir -p bin/assets/shaders

echo "Compiling shaders..."

echo "assets/shaders/BuiltinObjectShader.vert.glsl -> bin/assets/shaders/BuiltinObjectShader.vert.spv"
$VULKAN_SDK/bin/glslc -fshader-stage=vert assets/shaders/BuiltinObjectShader.vert.glsl -o bin/assets/shaders/BuiltinObjectShader.vert.spv
ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0 ]
then
echo "Error:"$ERRORLEVEL && exit
fi

echo "assets/shaders/BuiltinObjectShader.frag.glsl -> bin/assets/shaders/BuiltinObjectShader.frag.spv"
$VULKAN_SDK/bin/glslc -fshader-stage=frag assets/shaders/BuiltinObjectShader.frag.glsl -o bin/assets/shaders/BuiltinObjectShader.frag.spv
ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0 ]
then
echo "Error:"$ERRORLEVEL && exit
fi

echo "Copying assets..."
echo cp -R "assets" "bin"
cp -R "assets" "bin"

echo "Done."
