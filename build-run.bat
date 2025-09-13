@echo off
setlocal enabledelayedexpansion

REM Settings
set PROJECT_NAME=my_qt_app
set BUILD_DIR=release
set QMAKE_PATH=qmake6
set MAKE_PATH=mingw32-make

echo Creating build directory...
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

echo Cleaning previous build...
%MAKE_PATH% clean 2>nul

echo Generating Makefile...
%QMAKE_PATH% -makefile ../Qt.pro
if %errorlevel% neq 0 (
    echo ❌ Error generating Makefile!
    exit /b 1
)

echo Compiling project...
%MAKE_PATH% 
if %errorlevel% equ 0 (
    echo ✅ Build completed successfully!
    
    REM Searching for executable file in current directory (release)
    if exist "%PROJECT_NAME%.exe" (
        echo 🚀 Launching application: %PROJECT_NAME%.exe
        start "" "%PROJECT_NAME%.exe"
    ) else (
        echo ❌ Executable file not found in %BUILD_DIR%!
        echo Searching for possible executable files:
        dir *.exe | findstr /v "\.o\|\.a\|\.dll\|\.obj"
        exit /b 1
    )
) else (
    echo ❌ Build error!
    exit /b 1
)