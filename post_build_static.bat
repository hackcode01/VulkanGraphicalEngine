@echo off

REM Run from root directory!
if not exist "%cd%\bin_static\assets\shaders\" mkdir "%cd%\bin_static\assets\shaders"

echo "Compiling shaders..."

echo "assets/shaders/BuiltinMaterialShader.vert.glsl -> bin_static/assets/shaders/BuiltinMaterialShader.vert.spv"
%VULKAN_SDK%\bin\glslc.exe -fshader-stage=vert assets/shaders/BuiltinMaterialShader.vert.glsl -o bin_static/assets/shaders/BuiltinMaterialShader.vert.spv
IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)

echo "assets/shaders/BuiltinMaterialShader.frag.glsl -> bin_static/assets/shaders/BuiltinMaterialShader.frag.spv"
%VULKAN_SDK%\bin\glslc.exe -fshader-stage=frag assets/shaders/BuiltinMaterialShader.frag.glsl -o bin_static/assets/shaders/BuiltinMaterialShader.frag.spv
IF %ERRORLEVEL% NEQ 0 (echo Error: %ERRORLEVEL% && exit)

echo "Copying assets..."
echo xcopy "assets" "bin_static\assets" /h /i /c /k /e /r /y
xcopy "assets" "bin_static\assets" /h /i /c /k /e /r /y

echo "Done."
