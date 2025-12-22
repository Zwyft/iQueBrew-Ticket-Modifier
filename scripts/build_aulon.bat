@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
cd /d D:\AntigravityProjects\iQueGithub\aulon

echo.
echo ============================================
echo Building aulon for Windows XP (32-bit)...
echo ============================================
echo.

cl /Fe:aulon.exe /MT /D_USING_V110_SDK71_ /I "D:\AntigravityProjects\iQueGithub\libusb\include" /I "D:\AntigravityProjects\iQueGithub\libusb\include\libusb-1.0" src\main.c src\commands.c src\fs.c src\io.c src\menu.c src\menu_func.c src\player_comms.c src\usb.c src\usb_log.c src\server.c /link /SUBSYSTEM:CONSOLE,5.01 "D:\AntigravityProjects\iQueGithub\libusb\VS2013\MS32\dll\libusb-1.0.lib" Advapi32.lib Ws2_32.lib /FORCE:MULTIPLE

echo.
echo Copying MinGW XP-compatible libusb-1.0.dll...
copy "D:\AntigravityProjects\iQueGithub\libusb\MinGW32\dll\libusb-1.0.dll" . >nul
echo.

echo.
echo ============================================
echo Build finished. Checking for output...
echo ============================================
if exist aulon.exe (
    echo SUCCESS: aulon.exe was created!
    dir aulon.exe
) else (
    echo FAILED: aulon.exe was NOT created.
)
pause
