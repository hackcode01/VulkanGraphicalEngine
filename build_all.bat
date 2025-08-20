mkdir bin

ECHO "Building everything..."

@REM PUSHD engine
@REM CALL build.bat
@REM POPD
@REM IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exist)

@REM PUSHD editor
@REM CALL build.bat
@REM POPD
@REM IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

REM Engine
make -f "Makefile.engine.windows.mak" all
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

REM Editor
make -f "Makefile.editor.windows.mak" all
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

REM Tests
make -f "Makefile.tests.windows.mak" all
IF %ERRORLEVEL% NEQ 0 (echo Error:%ERRORLEVEL% && exit)

ECHO "All assemblies built successfully."
