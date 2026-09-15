@echo off
setlocal enabledelayedexpansion

:: ===================================================
::   Gality Engine - PRODUCT Build Script
::   (Debug tools disabled, optimized for release)
:: ===================================================

set BUILD_TYPE=Release
set BUILD_MODE=PRODUCT
set BUILD_DIR=build
set DIST_DIR=dist
set DEV_FLAG=OFF

echo ===================================================
echo   Gality Engine - PRODUCT Build
echo   Build dir: %BUILD_DIR%
echo   Output:    %DIST_DIR%
echo ===================================================
echo.

:: ===================================================
:: Step 1: Compile .gality DSL to JSON
:: ===================================================
echo [1/6] Compiling .gality DSL script...
python -m devtools.scripts.gality_compiler ^
    assets/scripts/main_story_multi.gality ^
    assets/scripts/demo_long.json
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile DSL script!
    pause
    exit /b %ERRORLEVEL%
)
echo   OK
echo.

:: ===================================================
:: Step 2: Pack assets into data.pak
:: ===================================================
echo [2/6] Packing assets into data.pak...
python -m devtools.scripts.gality_packer assets data.pak
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to pack assets!
    pause
    exit /b %ERRORLEVEL%
)
echo   OK
echo.

:: ===================================================
:: Step 3: Install vcpkg dependencies
:: ===================================================
echo [3/6] Installing vcpkg dependencies...
if not exist "vcpkg\vcpkg.exe" (
    echo [WARN] vcpkg.exe not found, bootstrapping...
    if not exist "vcpkg" (
        git clone https://github.com/microsoft/vcpkg.git
    )
    call vcpkg\bootstrap-vcpkg.bat
)
call vcpkg\vcpkg install
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to install dependencies!
    pause
    exit /b %ERRORLEVEL%
)
echo   OK
echo.

:: ===================================================
:: Step 4: CMake configure
:: ===================================================
echo [4/6] Configuring CMake (PRODUCT mode)...
cmake -B %BUILD_DIR% ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DGALITY_DEV_BUILD=%DEV_FLAG% ^
    -DCMAKE_TOOLCHAIN_FILE="%CD%/vcpkg/scripts/buildsystems/vcpkg.cmake"
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake configuration failed!
    pause
    exit /b %ERRORLEVEL%
)
echo   OK
echo.

:: ===================================================
:: Step 5: Build C++ engine
:: ===================================================
echo [5/6] Building C++ Engine (PRODUCT)...
cmake --build %BUILD_DIR% --config %BUILD_TYPE%
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] C++ build failed!
    pause
    exit /b %ERRORLEVEL%
)
echo   OK
echo.

:: ===================================================
:: Step 6: Assemble distribution package
:: ===================================================
echo [6/6] Generating PRODUCT package in .\%DIST_DIR%\...
if exist %DIST_DIR% rmdir /S /Q %DIST_DIR%
mkdir %DIST_DIR%

:: Copy data.pak
if exist data.pak (
    copy /Y data.pak %DIST_DIR%\
) else (
    echo [ERROR] data.pak not found!
    pause
    exit /b 1
)

:: Copy executable
if exist %BUILD_DIR%\%BUILD_TYPE%\Gality.exe (
    copy /Y %BUILD_DIR%\%BUILD_TYPE%\Gality.exe %DIST_DIR%\
) else if exist %BUILD_DIR%\Gality.exe (
    copy /Y %BUILD_DIR%\Gality.exe %DIST_DIR%\
) else (
    echo [ERROR] Gality.exe not found in %BUILD_DIR%!
    pause
    exit /b 1
)

:: Copy vcpkg runtime DLLs
if exist %BUILD_DIR%\vcpkg_installed\x64-windows\bin (
    echo   Copying vcpkg DLLs...
    copy /Y %BUILD_DIR%\vcpkg_installed\x64-windows\bin\*.dll %DIST_DIR%\
) else if exist vcpkg\installed\x64-windows\bin (
    echo   Copying vcpkg DLLs...
    copy /Y vcpkg\installed\x64-windows\bin\*.dll %DIST_DIR%\
)

:: Copy Visual C++ runtime DLLs
if exist "C:\Windows\System32\vcruntime140.dll" (
    echo   Copying VC++ runtime DLLs...
    copy /Y "C:\Windows\System32\vcruntime140.dll" %DIST_DIR%\ >nul 2>&1
    copy /Y "C:\Windows\System32\vcruntime140_1.dll" %DIST_DIR%\ >nul 2>&1
    copy /Y "C:\Windows\System32\msvcp140.dll" %DIST_DIR%\ >nul 2>&1
    copy /Y "C:\Windows\System32\msvcp140_1.dll" %DIST_DIR%\ >nul 2>&1
    copy /Y "C:\Windows\System32\msvcp140_2.dll" %DIST_DIR%\ >nul 2>&1
    copy /Y "C:\Windows\System32\concrt140.dll" %DIST_DIR%\ >nul 2>&1
)

:: Product build: DO NOT copy raw assets folder
:: All assets are in data.pak
echo   Skipping raw assets (packed in data.pak)

echo   OK
echo.

:: ===================================================
:: Generate version file
:: ===================================================
echo Generating build info...
(
    echo Gality Engine
    echo Build: PRODUCT
    echo Date:  %DATE% %TIME%
    echo Version: v0.2.0
) > %DIST_DIR%\BUILD_INFO.txt

:: ===================================================
:: Done
:: ===================================================
echo ===================================================
echo   SUCCESS: PRODUCT build complete
echo   Package: .\%DIST_DIR%\
echo.
echo   Contents:
dir /B %DIST_DIR%
echo.
echo   This package is ready for distribution.
echo   All assets are packed into data.pak.
echo ===================================================
pause