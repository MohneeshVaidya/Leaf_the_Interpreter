@echo off
set "OUTPUT_NAME=leaf.exe"

:: 1. Compile all .c files in the current directory
clang *.c -o "%OUTPUT_NAME%"

:: 2. Check for compilation errors
if %errorlevel% equ 0 (
    echo Compilation successful! Running...
    echo -----------------------------------
    "%OUTPUT_NAME%"
) else (
    echo Compilation failed.
)
pause
