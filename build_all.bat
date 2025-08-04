@REM @ECHO OFF
@REM REM Build everything

@REM ECHO "Building everything..."

@REM PUSHD engine
@REM CALL build.bat
@REM POPD
@REM IF %ERROR_LEVELS% NEQ 0 (echo Error:%ERROR_LEVELS% && exit)

@REM PUSHD editor
@REM CALL build.bat
@REM POPD
@REM IF %ERROR_LEVELS% NEQ 0 (echo Error:%ERROR_LEVELS% && exit)

@REM ECHO "All assemblies build successfully."

cd .\engine\ && .\build.bat && cd ../editor\ && ./build.batbuild.bat
