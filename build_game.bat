@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo   Gality Engine - Automated Build and Packager
echo ===================================================

:: 1. 編譯 DSL 劇本為 JSON
echo [1/5] Compiling .gality DSL script...
python -m devtools.scripts.gality_compiler assets/scripts/main_story_multi.gality assets/scripts/demo_long.json
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile DSL script!
    pause
    exit /b %ERRORLEVEL%
)

:: 2. 打包加密 assets 為 data.pak
echo [2/5] Packing assets into data.pak...
python -m devtools.scripts.gality_packer assets data.pak
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to pack assets!
    pause
    exit /b %ERRORLEVEL%
)

:: 3. 執行 CMake Release 建置
echo [3/5] Building C++ Engine (Release Mode)...
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] C++ Build failed!
    pause
    exit /b %ERRORLEVEL%
)

:: 4. 建立獨立發布資料夾並收集 DLL
echo [4/5] Generating Standalone Release Package...
if not exist dist mkdir dist

if exist data.pak (
    copy /Y data.pak dist\
)

if exist build\Release\Gality.exe (
    copy /Y build\Release\Gality.exe dist\
) else if exist build\Gality.exe (
    copy /Y build\Gality.exe dist\
)

:: 複製 vcpkg 的 DLL
if exist build\vcpkg_installed\x64-windows\bin (
    echo Copying runtime DLLs from vcpkg...
    copy /Y build\vcpkg_installed\x64-windows\bin\*.dll dist\
) else if exist build\vcpkg_installed\x64-windows\debug\bin (
    echo Copying debug runtime DLLs from vcpkg...
    copy /Y build\vcpkg_installed\x64-windows\debug\bin\*.dll dist\
)

:: 5. 複製 Visual C++ 執行階段 DLL (修復 VSMCP140.dll 缺失)
echo [5/5] Copying Visual C++ Runtime DLLs...
set "VC_REDIST=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC"

:: 自動尋找最新版本
set "VC_LATEST="
for /f "delims=" %%i in ('dir /b /ad /o-n "%VC_REDIST%" 2^>nul') do (
    if not defined VC_LATEST set "VC_LATEST=%%i"
)

if defined VC_LATEST (
    set "CRT_DIR=%VC_REDIST%\!VC_LATEST!\x64\Microsoft.VC143.CRT"
    if exist "!CRT_DIR!" (
        echo Copying CRT DLLs from !CRT_DIR!...
        copy /Y "!CRT_DIR!\*.dll" dist\
    ) else (
        echo [WARN] CRT directory not found: !CRT_DIR!
    )
) else (
    echo [WARN] Visual Studio Redist not found, trying System32...
    if exist "C:\Windows\System32\vcruntime140.dll" (
        copy /Y "C:\Windows\System32\vcruntime140.dll" dist\
    )
    if exist "C:\Windows\System32\vcruntime140_1.dll" (
        copy /Y "C:\Windows\System32\vcruntime140_1.dll" dist\
    )
    if exist "C:\Windows\System32\msvcp140.dll" (
        copy /Y "C:\Windows\System32\msvcp140.dll" dist\
    )
    if exist "C:\Windows\System32\msvcp140_1.dll" (
        copy /Y "C:\Windows\System32\msvcp140_1.dll" dist\
    )
    if exist "C:\Windows\System32\msvcp140_2.dll" (
        copy /Y "C:\Windows\System32\msvcp140_2.dll" dist\
    )
)

echo ===================================================
echo SUCCESS: Standalone package generated in .\dist
echo ===================================================
pause