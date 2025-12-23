@echo off
echo ============================================
echo    iQue Player Driver Installation Helper
echo ============================================
echo.
echo This script will help you install the iQue Player driver.
echo.
echo IMPORTANT: Before running this, you MUST disable driver 
echo signature enforcement first!
echo.
echo To disable driver signature enforcement:
echo 1. Open Settings ^> Update ^& Security ^> Recovery
echo 2. Click "Restart now" under Advanced Startup
echo 3. Choose: Troubleshoot ^> Advanced Options ^> Startup Settings
echo 4. Click Restart
echo 5. Press F7 to "Disable driver signature enforcement"
echo.
echo After Windows restarts, run this script again.
echo.
pause

echo.
echo Opening Device Manager...
echo Look for "Unknown device" or "iQue Player" under Other Devices.
echo Right-click ^> Update driver ^> Browse my computer ^> 
echo Let me pick ^> Have Disk ^> Browse to this folder.
echo.
start devmgmt.msc

echo.
echo Driver INF file location:
echo %~dp0ique_player.inf
echo.
pause
