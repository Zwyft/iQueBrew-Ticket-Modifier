@echo off
if not defined DevEnvDir call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"

set LIBUSB_DIR=D:\AntigravityProjects\iQueGithub\aulon\libusb-1.0.23
set BUILD_DIR=D:\AntigravityProjects\iQueGithub\aulon\libusb_build

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

echo Cleaning up old files...
del aulon.exe 2>nul
del libusb-1.0.dll 2>nul
del libusb-1.0.lib 2>nul
del libusb_xp.lib 2>nul


echo ==============================================
echo Compiling libusb 1.0.23 from source for XP...
echo ==============================================

set CFLAGS=/MT /nologo /D_WIN32_WINNT=0x0501 /DWINVER=0x0501 /D_USING_V110_SDK71_ /DOS_WINDOWS /DWIN32_LEAN_AND_MEAN /I"%LIBUSB_DIR%\libusb" /I"%LIBUSB_DIR%\msvc"

rem Compile libusb sources
cl %CFLAGS% /c /Fo"%BUILD_DIR%\core.obj" "%LIBUSB_DIR%\libusb\core.c"
cl %CFLAGS% /c /Fo"%BUILD_DIR%\descriptor.obj" "%LIBUSB_DIR%\libusb\descriptor.c"
cl %CFLAGS% /c /Fo"%BUILD_DIR%\hotplug.obj" "%LIBUSB_DIR%\libusb\hotplug.c"
cl %CFLAGS% /c /Fo"%BUILD_DIR%\io.obj" "%LIBUSB_DIR%\libusb\io.c"
cl %CFLAGS% /c /Fo"%BUILD_DIR%\strerror.obj" "%LIBUSB_DIR%\libusb\strerror.c"
cl %CFLAGS% /c /Fo"%BUILD_DIR%\sync.obj" "%LIBUSB_DIR%\libusb\sync.c"
cl %CFLAGS% /c /Fo"%BUILD_DIR%\poll_windows.obj" "%LIBUSB_DIR%\libusb\os\poll_windows.c"
cl %CFLAGS% /c /Fo"%BUILD_DIR%\threads_windows.obj" "%LIBUSB_DIR%\libusb\os\threads_windows.c"
cl %CFLAGS% /c /Fo"%BUILD_DIR%\windows_nt_common.obj" "%LIBUSB_DIR%\libusb\os\windows_nt_common.c"
cl %CFLAGS% /c /Fo"%BUILD_DIR%\windows_winusb.obj" "%LIBUSB_DIR%\libusb\os\windows_winusb.c"
cl %CFLAGS% /c /Fo"%BUILD_DIR%\windows_usbdk.obj" "%LIBUSB_DIR%\libusb\os\windows_usbdk.c"

echo.
echo Creating static library libusb_xp.lib...
lib /nologo /OUT:libusb_xp.lib "%BUILD_DIR%\*.obj"

echo.
echo ==============================================
echo Compiling aulon with libusb_xp.lib...
echo ==============================================

cd /d D:\AntigravityProjects\iQueGithub\aulon

cl /Fe:aulon_fixed.exe /MT /DLIBUSB_STATIC /D_USING_V110_SDK71_ /I "%LIBUSB_DIR%\libusb" src\main.c src\commands.c src\fs.c src\io.c src\menu.c src\menu_func.c src\player_comms.c src\usb.c src\usb_log.c src\server.c /link /SUBSYSTEM:CONSOLE,5.01 libusb_xp.lib Advapi32.lib Ws2_32.lib setupapi.lib /FORCE:MULTIPLE

echo.
if exist aulon_fixed.exe echo SUCCESS: aulon_fixed.exe built!

