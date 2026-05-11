@echo off
setlocal

echo ================================================
echo  giraphics ^| Build ^& Run
echo ================================================

:: ── Step 1: Configure (only if build folder doesn't exist) ──────────────
if not exist build (
    echo [1/3] Configuring project...
    cmake -S . -B build
    if errorlevel 1 (
        echo.
        echo [ERROR] CMake configure failed. Check the output above.
        pause
        exit /b 1
    )
) else (
    echo [1/3] Build folder exists, skipping configure.
)

:: ── Step 2: Build ────────────────────────────────────────────────────────
echo [2/3] Building...
cmake --build build --config Debug
if errorlevel 1 (
    echo.
    echo [ERROR] Build failed. Check the output above.
    pause
    exit /b 1
)

:: ── Step 3: Run ──────────────────────────────────────────────────────────
echo [3/3] Running Sandbox...
echo.
.\build\Sandbox\Debug\4-Vulkan-Debugging-Capabilities.exe

echo.
echo ================================================
echo  Done.
echo ================================================
pause
