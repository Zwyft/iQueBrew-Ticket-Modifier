@echo off
title iQue Tools - Complete Launcher
echo ============================================
echo    iQue Tools - Complete Launcher
echo ============================================
echo.

set VBOXMANAGE="C:\Program Files\Oracle\VirtualBox\VBoxManage.exe"
set VMNAME=Windows XP
set GUI_DIR=D:\AntigravityProjects\iQueGithub\gui

:: Check if VM is running
%VBOXMANAGE% showvminfo "%VMNAME%" --machinereadable 2>nul | findstr /C:"VMState=\"running\"" >nul
if %ERRORLEVEL%==0 (
    echo [OK] Windows XP VM is already running
) else (
    echo [..] Starting Windows XP VM...
    %VBOXMANAGE% startvm "%VMNAME%" --type headless
    echo [OK] VM started in headless mode
    echo [..] Waiting for VM to boot (30 seconds)...
    timeout /t 30 /nobreak >nul
)

echo.
echo [..] Installing Node.js dependencies...
cd /d "%GUI_DIR%"
if not exist node_modules (
    call npm install
)

echo.
echo [..] Starting bridge server...
start "iQue Tools Bridge Server" cmd /k "cd /d %GUI_DIR% && npm start"

:: Wait for server to start
timeout /t 3 /nobreak >nul

echo.
echo ============================================
echo    iQue Tools is ready!
echo ============================================
echo.
echo    Web GUI: http://localhost:8080
echo.
echo    IMPORTANT: In the XP VM, start aulon:
echo      aulon.exe -s 5001
echo.
echo    Then open your browser to:
echo      http://localhost:8080
echo.
echo ============================================

:: Open browser
start http://localhost:8080

pause
