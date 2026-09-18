@echo off
setlocal enabledelayedexpansion

:: ============================================================================
::   Gality Benchmark - Installation Script
::
::   This script sets up the benchmark environment by:
::     1. Verifying the main Gality project exists
::     2. Creating vcpkg symbolic link (or copying if symlink fails)
::     3. Copying data.pak and assets from the main project
::     4. Compiling the performance test script
::     5. Verifying the toolchain
::
::   Prerequisites:
::     - Gality main project must be cloned at ..\Gality\
::     - Gality main project must be built at least once (build_dev.bat)
::     - Python 3.8+ in PATH
::     - Visual Studio 2022 with C++ tools
::
::   Usage:
::     install.bat
:: ============================================================================

cd /d "%~dp0"

set SCRIPT_DIR=%~dp0
if "%SCRIPT_DIR:~-1%"=="\" set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%

set GALITY_ROOT=%SCRIPT_DIR%\..\Gality

set START_TIME=%TIME%

echo.
echo ===================================================
echo   Gality Benchmark - Installation
echo ===================================================
echo   Benchmark dir: %SCRIPT_DIR%
echo   Gality root:   %GALITY_ROOT%
echo   Started:       %START_TIME%
echo ===================================================
echo.

:: ============================================================================
:: Step 1: Verify main Gality project
:: ============================================================================
echo [1/6] Verifying main Gality project...

if not exist "%GALITY_ROOT%" (
    echo [ERROR] Main Gality project not found.
    echo         Expected at: %GALITY_ROOT%
    echo.
    echo   Please:
    echo     1. Clone Gality:  git clone https://github.com/moli0516/Gality.git
    echo     2. Run Gality installer:  cd Gality ^&^& install.bat
    echo     3. Then re-run this script.
    echo.
    pause
    exit /b 1
)

if not exist "%GALITY_ROOT%\src" (
    echo [ERROR] Invalid Gality project at %GALITY_ROOT%
    echo         Missing: src\ directory
    pause
    exit /b 1
)

if not exist "%GALITY_ROOT%\vcpkg\vcpkg.exe" (
    echo [ERROR] Gality vcpkg not installed.
    echo         Expected: %GALITY_ROOT%\vcpkg\vcpkg.exe
    echo.
    echo   Please run the Gality installer first:
    echo     cd %GALITY_ROOT%
    echo     install.bat
    echo.
    pause
    exit /b 1
)

echo   OK - Gality project verified
echo.

:: ============================================================================
:: Step 2: Setup vcpkg (symbolic link or copy)
:: ============================================================================
echo [2/6] Setting up vcpkg...

if exist "vcpkg" (
    :: Check if it's a symlink or a real directory
    dir /AL "vcpkg" >nul 2>&1
    if %ERRORLEVEL% EQU 0 (
        echo   OK - vcpkg symlink already exists
    ) else (
        echo   OK - vcpkg directory already exists (standalone)
    )
) else (
    echo   Attempting symbolic link to main project's vcpkg...
    mklink /D vcpkg "%GALITY_ROOT%\vcpkg" >nul 2>&1
    if %ERRORLEVEL% EQU 0 (
        echo   OK - vcpkg symlink created
        echo        Target: %GALITY_ROOT%\vcpkg
    ) else (
        echo   [WARN] Symlink creation failed (needs admin or developer mode)
        echo          Falling back to full copy...
        echo          This will take a moment.
        xcopy /E /I /Y /Q "%GALITY_ROOT%\vcpkg" vcpkg >nul
        if %ERRORLEVEL% NEQ 0 (
            echo [ERROR] Failed to copy vcpkg!
            echo.
            echo   Try one of these options:
            echo     1. Run this script as Administrator (for symlink)
            echo     2. Enable Developer Mode in Windows Settings
            echo     3. Manually copy %GALITY_ROOT%\vcpkg to this directory
            pause
            exit /b 1
        )
        echo   OK - vcpkg copied
    )
)
echo.

:: ============================================================================
:: Step 3: Copy data.pak
:: ============================================================================
echo [3/6] Copying data.pak...

if not exist "%GALITY_ROOT%\data.pak" (
    echo [WARN] data.pak not found in main project.
    echo        Build Gality first: cd %GALITY_ROOT% ^&^& build_dev.bat
    echo        Skipping for now - run build.bat later to generate it.
) else (
    copy /Y "%GALITY_ROOT%\data.pak" data.pak >nul
    echo   OK - data.pak copied
)
echo.

:: ============================================================================
:: Step 4: Copy assets (for reference)
:: ============================================================================
echo [4/6] Copying assets...

if exist assets (
    echo   OK - assets directory already exists
) else (
    if exist "%GALITY_ROOT%\assets" (
        xcopy /E /I /Y /Q "%GALITY_ROOT%\assets" assets >nul
        echo   OK - assets copied
    ) else (
        echo   [WARN] assets not found in main project
    )
)
echo.

:: ============================================================================
:: Step 5: Compile performance test script
:: ============================================================================
echo [5/6] Compiling performance test script...

if not exist "scripts\perf_test.gality" (
    echo   [WARN] scripts\perf_test.gality not found
    echo          Create it first, then re-run this script.
    goto :skip_compile
)

python --version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo   [WARN] Python not found in PATH
    echo          Install Python 3.8+ or add it to PATH
    goto :skip_compile
)

python "%GALITY_ROOT%\devtools\scripts\gality_compiler.py" ^
    "scripts\perf_test.gality" ^
    "%GALITY_ROOT%\assets\scripts\demo_long.json"
if %ERRORLEVEL% NEQ 0 (
    echo   [ERROR] Failed to compile perf_test.gality
    pause
    exit /b 1
)
echo   OK - perf_test.gality compiled
:skip_compile
echo.

:: ============================================================================
:: Step 6: Final checks
:: ============================================================================
echo [6/6] Final checks...

set MISSING=0

if not exist "CMakeLists.txt" (
    echo   [WARN] CMakeLists.txt missing
    set MISSING=1
)

if not exist "src\main_benchmark.cpp" (
    echo   [WARN] src\main_benchmark.cpp missing
    set MISSING=1
)

if not exist "src\PerfMetrics.hpp" (
    echo   [WARN] src\PerfMetrics.hpp missing
    set MISSING=1
)

if %MISSING%==0 (
    echo   OK - All expected files present
) else (
    echo.
    echo   [WARN] Some files are missing.
    echo          The build may fail until these are added.
)
echo.

:: ============================================================================
:: Done
:: ============================================================================
set END_TIME=%TIME%

echo ===================================================
echo   SUCCESS: Benchmark setup complete
echo ===================================================
echo.
echo   Started: %START_TIME%
echo   Ended:   %END_TIME%
echo.
echo   Next steps:
echo     1. Ensure src\*.hpp and src\*.cpp exist
echo     2. Build:  build.bat
echo     3. Run:    run.bat
echo.
echo ===================================================

powershell -c "[console]::beep(800,200)" 2>nul
pause