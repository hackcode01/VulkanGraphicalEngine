@ECHO OFF
REM Build everything

ECHO "Building everything..."

PUSHD engine
CALL build.bat
POPD
IF %ERROR_LEVELS% NEQ 0 (echo Error:%ERROR_LEVELS% && exit)

PUSHD editor
CALL build.bat
POPD
IF %ERROR_LEVELS% NEQ 0 (echo Error:%ERROR_LEVELS% && exit)

ECHO "All assemblies build successfully."
