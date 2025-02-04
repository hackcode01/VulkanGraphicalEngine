REM Build script for editor
@ECHO OFF
SetLocal EnableDelayedExpansion

REM Get a list of all the .c files.
SET cFilenames=
FOR /R %%f in (*.c) do (
    SET cFilenames=!cFilenames! %%f
)

REM echo "Files:" %cFilenames%

SET assembly=editor
SET compilerFlags=-g 
REM -Wall -Werror
SET includeFlags=-Isrc -I../engine/src/
SET linkerFlags=-L../build/ -lengine.lib
SET defines=-D_DEBUG -DIMPORT

ECHO "Building %assembly%%..."
clang %cFilenames% %compilerFlags% -o ../build/%assembly%.exe %defines% %includeFlags% %linkerFlags%
