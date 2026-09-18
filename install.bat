@echo off
setlocal enabledelayedexpansion

:: ============================================================================
::
::   Gality Engine - Installation Script
::
::   This script sets up the development environment by:
::     1. Verifying Python 3.8+
::     2. Verifying Git
::     3. Bootstrapping vcpkg (clone + bootstrap)
::     4. Installing C++ dependencies (sfml, nlohmann-json)
::     5. Verifying CMake toolchain
::
::   Prerequisites:
::     - Visual Studio 2022 with C++ desktop development workload
::     - Git in PATH
::     - Python 3.8+ in PATH
::
::   Usage:
::     install.bat
::
::   After installation:
::     build_dev.bat    - develop build (debug tools enabled)
::     build_prod.bat   - production build (optimized release)
:: ============================================================================

cd /d "%~dp0"

set SCRIPT_DIR=%~dp0
if "%SCRIPT_DIR:~-1%"=="\" set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%

set START_TIME=%TIME%

echo.
echo ===================================================
echo   Gality Engine - Installation
echo ===================================================
echo   Directory: %SCRIPT_DIR%
echo   Started:   %START_TIME%
echo ===================================================
echo.

:: ============================================================================
:: Step 1: Verify Python
:: ============================================================================
echo [1/5] Verifying Python...

python --version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Python not found in PATH.
    echo.
    echo   Install Python 3.8+ from:
    echo     https://www.python.org/downloads/
    echo.
    echo   Make sure to check "Add Python to PATH" during installation.
    powershell -c "[console]::beep(300,500)" 2>nul
    pause
    exit /b 1
)

for /f "tokens=2" %%v in ('python --version 2^>^&1') do set PY_VERSION=%%v
echo   OK - Python %PY_VERSION%
echo.

:: ============================================================================
:: Step 2: Verify Git
:: ============================================================================
echo [2/5] Verifying Git...

git --version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Git not found in PATH.
    echo.
    echo   Install Git from:
    echo     https://git-scm.com/downloads
    powershell -c "[console]::beep(300,500)" 2>nul
    pause
    exit /b 1
)

for /f "tokens=3" %%v in ('git --version') do set GIT_VERSION=%%v
echo   OK - Git %GIT_VERSION%
echo.

:: ============================================================================
:: Step 3: Bootstrap vcpkg
:: ============================================================================
echo [3/5] Setting up vcpkg...

if exist "vcpkg\vcpkg.exe" (
    echo   OK - vcpkg already bootstrapped
) else (
    if not exist "vcpkg" (
        echo   Cloning vcpkg from GitHub...
        git clone https://github.com/microsoft/vcpkg.git
        if %ERRORLEVEL% NEQ 0 (
            echo [ERROR] Failed to clone vcpkg!
            powershell -c "[console]::beep(300,500)" 2>nul
            pause
            exit /b 1
        )
    )

    echo   Bootstrapping vcpkg...
    call vcpkg\bootstrap-vcpkg.bat
    if %ERRORLEVEL% NEQ 0 (
        echo [ERROR] vcpkg bootstrap failed!
        powershell -c "[console]::beep(300,500)" 2>nul
        pause
        exit /b 1
    )
)
echo   OK
echo.

:: ============================================================================
:: Step 4: Install C++ dependencies
:: ============================================================================
echo [4/5] Installing C++ dependencies (sfml, nlohmann-json)...
echo        This may take 5-15 minutes on first run.
echo.

call vcpkg\vcpkg install
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Dependency installation failed!
    echo.
    echo   Try running manually:
    echo     vcpkg\vcpkg install
    powershell -c "[console]::beep(300,500)" 2>nul
    pause
    exit /b 1
)
echo   OK
echo.

:: ============================================================================
:: Step 5: Verify CMake
:: ============================================================================
echo [5/5] Verifying CMake...

where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo   [WARN] CMake not found in PATH.
    echo          Install CMake 3.20+ from:
    echo            https://cmake.org/download/
    echo.
    echo          The build scripts will fail until CMake is available.
) else (
    for /f "tokens=3" %%v in ('cmake --version') do set CMAKE_VERSION=%%v
    echo   OK - CMake !CMAKE_VERSION!
)
echo.

:: ============================================================================
:: Done
:: ============================================================================
set END_TIME=%TIME%

echo ===================================================
echo   SUCCESS: Gality development environment ready
echo ===================================================
echo   Started: %START_TIME%
echo   Ended:   %END_TIME%
echo.
echo   Next steps:
echo     1. Build (dev):   build_dev.bat
echo     2. Build (prod):  build_prod.bat
echo     3. Run:           cd dist-dev ^&^& gality.exe
echo.
echo   The build script will:
echo     - Compile .gality DSL to JSON AST
echo     - Validate the compiled script
echo     - Pack assets into data.pak
echo     - Build the C++ engine
echo     - Assemble a standalone bundle
echo ===================================================

powershell -c "[console]::beep(800,200)" 2>nul
pause