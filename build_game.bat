@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo   Gality Engine - Automated Build and Packager
echo ===================================================

:: 1. 編譯 DSL 劇本為 JSON[cite: 1]
echo [1/4] Compiling .gality DSL script...
python gality_compiler.py assets/scripts/demo_long.gality assets/scripts/demo_long.json
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile DSL script!
    pause
    exit /b %ERRORLEVEL%
)

:: 2. 打包加密 assets 為 data.pak[cite: 1]
echo [2/4] Packing assets into data.pak...
python gality_packer.py assets data.pak
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

:: 複製執行檔與 DLL[cite: 1]
if exist build\Release\Gality.exe (
    copy /Y build\Release\Gality.exe dist\
) else if exist build\Gality.exe (
    copy /Y build\Gality.exe dist\
)

if exist build\vcpkg_installed\x64-windows\bin (
    copy /Y build\vcpkg_installed\x64-windows\bin\*.dll dist\
)

:: 💡 直接複製完整的 assets 資料夾至 dist\
xcopy /E /I /Y assets dist\assets

:: 💡 自動尋找並複製 vcpkg 下所有的第三方需求 DLL (SFML 等)[cite: 1]
if exist build\vcpkg_installed\x64-windows\bin (
    echo Copying runtime DLLs from vcpkg...
    copy /Y build\vcpkg_installed\x64-windows\bin\*.dll dist\
) else if exist build\vcpkg_installed\x64-windows\debug\bin (
    echo Copying debug runtime DLLs from vcpkg...
    copy /Y build\vcpkg_installed\x64-windows\debug\bin\*.dll dist\
)

:: 複製外置 UI Theme 配置檔[cite: 1]
if exist assets\config\ui_theme.json (
    if not exist dist\assets\config mkdir dist\assets\config
    copy /Y assets\config\ui_theme.json dist\assets\config\
)

echo ===================================================
echo SUCCESS: Standalone package generated in .\dist
echo ===================================================
pause