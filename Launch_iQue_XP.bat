@echo off
title iQue Player - XP VM Launcher
echo ============================================
echo    iQue Player - Quick XP VM Launcher
echo ============================================
echo.

set VBOXMANAGE="C:\Program Files\Oracle\VirtualBox\VBoxManage.exe"
set VMNAME=Windows XP

:: Check if VM is already running
%VBOXMANAGE% showvminfo "%VMNAME%" --machinereadable 2>nul | findstr /C:"VMState=\"running\"" >nul
if %ERRORLEVEL%==0 (
    echo VM is already running! Opening window...
    goto :showvm
)

echo Starting Windows XP VM...
echo.
echo TIP: Once the VM is running, you can use Seamless Mode:
echo      Press Right-Ctrl + L to toggle seamless mode
echo      (only the app windows will show, no XP desktop!)
echo.

:: Start VM in normal GUI mode (seamless mode can be toggled with Host+L)
%VBOXMANAGE% startvm "%VMNAME%" --type gui

echo.
echo VM started! 
echo.
echo INSTRUCTIONS:
echo 1. Wait for Windows XP to boot
echo 2. Plug in your iQue Player (USB will auto-attach)
echo 3. Run the iQue software inside the VM
echo 4. Press Right-Ctrl + L for seamless mode (optional)
echo.
echo To close: Just shut down Windows XP normally
echo.
pause
goto :eof

:showvm
:: If VM is running, just bring it to front
start "" "C:\Program Files\Oracle\VirtualBox\VirtualBox.exe" --startvm "%VMNAME%"
echo.
echo VM window should now be visible.
pause
