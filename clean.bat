@echo off
setlocal

echo ================================================
echo  giraphics ^| Clean Build
echo ================================================

if exist build (
    echo Deleting build folder...
    rmdir /s /q build
    echo Done. Run run.bat to rebuild from scratch.
) else (
    echo Nothing to clean — build folder does not exist.
)

echo.
pause
