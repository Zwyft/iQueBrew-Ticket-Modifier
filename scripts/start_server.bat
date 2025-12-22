@echo off
REM Script to start aulon in server mode inside Windows XP VM
REM This should be placed in the XP VM's shared folder

echo ============================================
echo    Aulon Server for iQue Tools GUI
echo ============================================
echo.
echo Starting aulon TCP server on port 5001...
echo The native GUI can now connect to this server.
echo.
echo Press Ctrl+C to stop the server.
echo ============================================
echo.

aulon.exe -s 5001
if errorlevel 1 (
    echo.
    echo Server exited with an error!
    pause
) else (
    echo.
    echo Server stopped.
    pause
)
