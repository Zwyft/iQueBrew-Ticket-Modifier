@echo off
setlocal EnableDelayedExpansion

REM Navigate to project root (parent of scripts folder)
pushd %~dp0..

REM Setup Visual Studio environment for Windows XP (v141_xp)

echo Detecting Visual Studio 2022...

if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" goto FOUND_COMMUNITY
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" goto FOUND_BUILDTOOLS

echo Visual Studio 2022 not found.
pause
exit /b 1

:FOUND_COMMUNITY
set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
echo Found VS at: Community
goto SETUP_ENV

:FOUND_BUILDTOOLS
set "VS_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
echo Found VS at: BuildTools
goto SETUP_ENV

:SETUP_ENV
echo Setting up XP Toolset (v141_xp)...
call "!VS_PATH!" x86 -vcvars_ver=14.16.27023
if errorlevel 1 (
   echo Failed to verify XP toolset availability.
   pause
   exit /b 1
)

set "XP_SDK_DIR=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A"
set "INCLUDE=%XP_SDK_DIR%\Include;%INCLUDE%"
set "LIB=%XP_SDK_DIR%\Lib;%LIB%"

echo XP SDK set to: %XP_SDK_DIR%

REM Compiler Options
REM /D_WIN32_WINNT=0x0501 : Target Windows XP
REM /D_WIN32_IE=0x0600    : Target IE 6.0 (Common Controls 6)
REM /D_USING_V110_SDK71_  : Required for XP toolset
REM /DGUI_BUILD          : Custom flag for interaction with gui_redirect.h
set "OPTS=/O2 /Ob1 /Oi /Ot /Oy /MD /D_WIN32_WINNT=0x0501 /D_WIN32_IE=0x0500 /D_USING_V110_SDK71_ /DPOINTER_64=__ptr64 /DGUI_BUILD"

REM Include Paths
set "INCLUDES=/I src /I libusb-1.0.23 /I libusb-1.0.23/libusb /I libusb-1.0.23/msvc /I gui"

REM Libraries
set "LIBS=ws2_32.lib setupapi.lib advapi32.lib comctl32.lib user32.lib gdi32.lib shell32.lib"

REM LibUSB Sources
set "LIBUSB_SRC=libusb-1.0.23\libusb"
set "LIBUSB_FILES=%LIBUSB_SRC%\core.c %LIBUSB_SRC%\descriptor.c %LIBUSB_SRC%\hotplug.c %LIBUSB_SRC%\io.c %LIBUSB_SRC%\strerror.c %LIBUSB_SRC%\sync.c %LIBUSB_SRC%\os\poll_windows.c %LIBUSB_SRC%\os\threads_windows.c %LIBUSB_SRC%\os\windows_nt_common.c %LIBUSB_SRC%\os\windows_winusb.c %LIBUSB_SRC%\os\windows_usbdk.c"

echo Compiling Resources...
rc /fo gui/resource.res gui/resource.rc

echo Compiling GUI...
cl /c %OPTS% %INCLUDES% gui\main_gui.cpp /Fo:gui\main_gui.obj
if errorlevel 1 (
   echo GUI Compilation Failed
   exit /b 1
)

echo Compiling C sources...
cl %OPTS% %INCLUDES% src\commands.c src\fs.c src\aulon_io.c src\menu_func.c src\player_comms.c src\usb.c src\usb_log.c %LIBUSB_FILES% gui/resource.res gui\main_gui.obj /Fe:dist\ique_home.exe /link %LIBS% /SUBSYSTEM:WINDOWS,5.01

if errorlevel 1 (
   echo BUILD FAILED
   exit /b 1
)


echo Compiling Modern GUI...
cl /c %OPTS% %INCLUDES% gui\modern_gui.cpp /Fo:gui\modern_gui.obj
if errorlevel 1 (
   echo Modern GUI Compilation Failed
   exit /b 1
)

echo Linking Modern GUI...
cl %OPTS% %INCLUDES% src\commands.c src\fs.c src\aulon_io.c src\menu_func.c src\player_comms.c src\usb.c src\usb_log.c %LIBUSB_FILES% gui/resource.res gui\modern_gui.obj /Fe:dist\ique_modern.exe /link %LIBS% /SUBSYSTEM:WINDOWS,5.01

if errorlevel 1 (
   echo BUILD FAILED
   exit /b 1
)

REM Cleanup .obj files from root
del *.obj 2>nul

echo SUCCESS: dist\ique_home.exe and dist\ique_modern.exe created.

popd
