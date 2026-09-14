@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo   Gality Engine - Automated Build and Packager
echo ===================================================

:: 1. 編譯 DSL 劇本為 JSON[cite: 1]
echo [1/4] Compiling .gality DSL script...
python -m devtools.scripts.gality_compiler assets/scripts/main_story_multi.gality assets/scripts/demo_long.json
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile DSL script!
    pause
    exit /b %ERRORLEVEL%
)

:: 2. 打包加密 assets 為 data.pak[cite: 1]
echo [2/4] Packing assets into data.pak...
python -m devtools.scripts.gality_packer assets data.pak
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to pack assets!
    pause
    exit /b %ERRORLEVEL%
)

:: 3. 執行 CMake Release 建置[cite: 1]
echo [3/4] Building C++ Engine (Release Mode)...
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] C++ Build failed!
    pause
    exit /b %ERRORLEVEL%
)

:: 4. 建立獨立發布資料夾並收集 DLL[cite: 1]
echo [4/4] Generating Standalone Release Package...
if not exist dist mkdir dist

:: 複製加密資源包，讓發布版可以直接從 data.pak 讀取資源
if exist data.pak (
    copy /Y data.pak dist\
)

:: 複製執行檔與 DLL[cite: 1]
if exist build\Release\Gality.exe (
    copy /Y build\Release\Gality.exe dist\
) else if exist build\Gality.exe (
    copy /Y build\Gality.exe dist\
)

if exist build\vcpkg_installed\x64-windows\bin (
    copy /Y build\vcpkg_installed\x64-windows\bin\*.dll dist\
)

:: Strict archive-only package: do not copy the raw assets directory.
:: The game reads from data.pak instead.

:: 💡 自動尋找並複製 vcpkg 下所有的第三方需求 DLL (SFML 等)[cite: 1]
if exist build\vcpkg_installed\x64-windows\bin (
    echo Copying runtime DLLs from vcpkg...
    copy /Y build\vcpkg_installed\x64-windows\bin\*.dll dist\
) else if exist build\vcpkg_installed\x64-windows\debug\bin (
    echo Copying debug runtime DLLs from vcpkg...
    copy /Y build\vcpkg_installed\x64-windows\debug\bin\*.dll dist\
)

echo ===================================================
echo SUCCESS: Standalone package generated in .\dist
echo ===================================================
pause