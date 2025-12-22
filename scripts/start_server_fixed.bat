@echo off
echo Starting aulon_xp TCP server...
aulon_xp.exe -s 5001
if errorlevel 1 (
    echo.
    echo Server exited with an error!
    pause
) else (
    echo.
    echo Server stopped.
    pause
)
