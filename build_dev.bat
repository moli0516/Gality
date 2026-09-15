@echo off
setlocal enabledelayedexpansion

:: ===================================================
::   Gality Engine - DEVELOP Build Script
::   Build Type: RelWithDebInfo (optimized + debug symbols)
:: ===================================================

:: 切換到腳本所在目錄
cd /d "%~dp0"

set SCRIPT_DIR=%~dp0
if "%SCRIPT_DIR:~-1%"=="\" set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%

:: ⚠️ 關鍵修改：改用 RelWithDebInfo
set BUILD_TYPE=RelWithDebInfo
set BUILD_MODE=DEVELOP
set BUILD_DIR=build-dev
set DIST_DIR=dist-dev
set DEV_FLAG=ON

set START_TIME=%TIME%

echo ===================================================
echo   Gality Engine - DEVELOP Build
echo   Build type: %BUILD_TYPE%
echo   Build dir:  %BUILD_DIR%
echo   Output:     %DIST_DIR%
echo   Started:    %START_TIME%
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
    powershell -c "[console]::beep(300,500)" 2>nul
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
    powershell -c "[console]::beep(300,500)" 2>nul
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
    powershell -c "[console]::beep(300,500)" 2>nul
    pause
    exit /b %ERRORLEVEL%
)
echo   OK
echo.

:: ===================================================
:: Step 4: CMake configure
:: ===================================================
echo [4/6] Configuring CMake (%BUILD_TYPE%)...
cmake -B %BUILD_DIR% ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DGALITY_DEV_BUILD=%DEV_FLAG% ^
    -DCMAKE_TOOLCHAIN_FILE="%SCRIPT_DIR%/vcpkg/scripts/buildsystems/vcpkg.cmake"
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake configuration failed!
    powershell -c "[console]::beep(300,500)" 2>nul
    pause
    exit /b %ERRORLEVEL%
)
echo   OK
echo.

:: ===================================================
:: Step 5: Build C++ engine
:: ===================================================
echo [5/6] Building C++ Engine (%BUILD_TYPE%)...
cmake --build %BUILD_DIR% --config %BUILD_TYPE% --parallel
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] C++ build failed!
    powershell -c "[console]::beep(300,500)" 2>nul
    pause
    exit /b %ERRORLEVEL%
)
echo   OK
echo.

:: ===================================================
:: Step 6: Assemble distribution package
:: ===================================================
echo [6/6] Generating DEVELOP package in .\%DIST_DIR%\...
if not exist %DIST_DIR% mkdir %DIST_DIR%

:: 清理舊檔案
del /Q %DIST_DIR%\Gality.exe 2>nul
del /Q %DIST_DIR%\data.pak 2>nul
del /Q %DIST_DIR%\*.dll 2>nul
del /Q %DIST_DIR%\*.pdb 2>nul

:: data.pak
if exist data.pak (
    copy /Y data.pak %DIST_DIR%\
)

:: ===================================================
:: 執行檔（多重路徑搜尋）
:: ===================================================
set EXE_PATH=
set PDB_PATH=

if exist "%BUILD_DIR%\%BUILD_TYPE%\Gality.exe" (
    set EXE_PATH=%BUILD_DIR%\%BUILD_TYPE%\Gality.exe
    if exist "%BUILD_DIR%\%BUILD_TYPE%\Gality.pdb" set PDB_PATH=%BUILD_DIR%\%BUILD_TYPE%\Gality.pdb
)
if "%EXE_PATH%"=="" if exist "%BUILD_DIR%\Release\Gality.exe" (
    set EXE_PATH=%BUILD_DIR%\Release\Gality.exe
    if exist "%BUILD_DIR%\Release\Gality.pdb" set PDB_PATH=%BUILD_DIR%\Release\Gality.pdb
)
if "%EXE_PATH%"=="" if exist "%BUILD_DIR%\Gality.exe" (
    set EXE_PATH=%BUILD_DIR%\Gality.exe
    if exist "%BUILD_DIR%\Gality.pdb" set PDB_PATH=%BUILD_DIR%\Gality.pdb
)

if "%EXE_PATH%"=="" (
    echo [ERROR] Gality.exe not found in %BUILD_DIR%!
    echo Searched:
    echo   %BUILD_DIR%\%BUILD_TYPE%\Gality.exe
    echo   %BUILD_DIR%\Release\Gality.exe
    echo   %BUILD_DIR%\Gality.exe
    powershell -c "[console]::beep(300,500)" 2>nul
    pause
    exit /b 1
)

echo   Copying executable: %EXE_PATH%
copy /Y "%EXE_PATH%" %DIST_DIR%\

:: 複製 PDB（除錯符號）
if not "%PDB_PATH%"=="" (
    echo   Copying PDB: %PDB_PATH%
    copy /Y "%PDB_PATH%" %DIST_DIR%\
)

:: ===================================================
:: vcpkg runtime DLLs
:: ===================================================
if exist "%BUILD_DIR%\vcpkg_installed\x64-windows\bin" (
    echo   Copying vcpkg DLLs from %BUILD_DIR%\vcpkg_installed\...
    copy /Y "%BUILD_DIR%\vcpkg_installed\x64-windows\bin\*.dll" %DIST_DIR%\ >nul
) else if exist vcpkg\installed\x64-windows\bin (
    echo   Copying vcpkg DLLs from vcpkg\installed\...
    copy /Y vcpkg\installed\x64-windows\bin\*.dll %DIST_DIR%\ >nul
) else (
    echo [WARN] vcpkg DLLs not found!
)

:: ===================================================
:: VC++ Runtime DLLs
:: ===================================================
if exist "C:\Windows\System32\vcruntime140.dll" (
    echo   Copying VC++ runtime DLLs...
    copy /Y "C:\Windows\System32\vcruntime140.dll" %DIST_DIR%\ >nul 2>&1
    copy /Y "C:\Windows\System32\vcruntime140_1.dll" %DIST_DIR%\ >nul 2>&1
    copy /Y "C:\Windows\System32\msvcp140.dll" %DIST_DIR%\ >nul 2>&1
    copy /Y "C:\Windows\System32\msvcp140_1.dll" %DIST_DIR%\ >nul 2>&1
    copy /Y "C:\Windows\System32\msvcp140_2.dll" %DIST_DIR%\ >nul 2>&1
    copy /Y "C:\Windows\System32\concrt140.dll" %DIST_DIR%\ >nul 2>&1
)

:: ===================================================
:: assets 資料夾（供熱重載）
:: ===================================================
if exist assets (
    echo   Copying assets for hot reload...
    xcopy /E /I /Y assets "%DIST_DIR%\assets" >nul
)

:: 設定檔
if exist assets\config (
    if not exist "%DIST_DIR%\assets\config" mkdir "%DIST_DIR%\assets\config"
    copy /Y assets\config\*.json "%DIST_DIR%\assets\config\" >nul 2>&1
)

echo   OK
echo.

:: ===================================================
:: Done
:: ===================================================
set END_TIME=%TIME%

echo ===================================================
echo   SUCCESS: DEVELOP build complete
echo   Build type: %BUILD_TYPE%
echo   Package:    .\%DIST_DIR%\
echo   Started:    %START_TIME%
echo   Ended:      %END_TIME%
echo.
echo   Debug shortcuts enabled:
echo     F1 / ~  -  DebugOverlay
echo     F2      -  NodeGraphViewer
echo     F5/F9   -  Quick Save / Load
echo     F12     -  Screenshot
echo     K       -  Test screen shake
echo.
echo   HotReload: edit .gality and recompile demo_long.json
echo ===================================================

powershell -c "[console]::beep(800,200)" 2>nul
pause