@echo off
setlocal EnableDelayedExpansion

echo Detecting Visual Studio 2022...
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
  set "VS_PATH=%%i"
)

if "%VS_PATH%"=="" (
  echo ERROR: Visual Studio 2022 not found!
  exit /b 1
)

echo Found VS at: %VS_PATH%
echo Setting up XP Toolset (v141_xp)...

rem Try 14.1 version for XP support
call "%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat" x86 -vcvars_ver=14.1
if errorlevel 1 (
   echo ERROR: Failed to setup v141 toolset. Is "C++ Windows XP Support" installed?
   exit /b 1
)

echo Toolset initialized.

set "XP_SDK_DIR=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A"
set "INCLUDE=%XP_SDK_DIR%\Include;%INCLUDE%"
set "LIB=%XP_SDK_DIR%\Lib;%LIB%"

echo XP SDK set to: %XP_SDK_DIR%
cl /Bv



rem Config for MSVC
set "LIBUSB_ROOT=libusb-1.0.23"
set "LIBUSB_SRC=%LIBUSB_ROOT%\libusb"
set "OPTS=/nologo /MD /O2 /D_WIN32_WINNT=0x0501 /D_USING_V110_SDK71_ /D_CRT_SECURE_NO_WARNINGS /DWIN32_LEAN_AND_MEAN /D_CONSOLE"
set "INCLUDES=/I src /I %LIBUSB_SRC% /I %LIBUSB_ROOT%\msvc"

echo Compiling aulon_xp.exe...
cl %OPTS% %INCLUDES% ^
  src\main.c ^
  src\commands.c ^
  src\fs.c ^
  src\aulon_io.c ^
  src\menu.c ^
  src\menu_func.c ^
  src\player_comms.c ^
  src\usb.c ^
  src\usb_log.c ^
  src\server.c ^
  %LIBUSB_SRC%\core.c ^
  %LIBUSB_SRC%\descriptor.c ^
  %LIBUSB_SRC%\hotplug.c ^
  %LIBUSB_SRC%\io.c ^
  %LIBUSB_SRC%\strerror.c ^
  %LIBUSB_SRC%\sync.c ^
  %LIBUSB_SRC%\os\poll_windows.c ^
  %LIBUSB_SRC%\os\threads_windows.c ^
  %LIBUSB_SRC%\os\windows_nt_common.c ^
  %LIBUSB_SRC%\os\windows_winusb.c ^
  %LIBUSB_SRC%\os\windows_usbdk.c ^
  /Fe:aulon_xp.exe ^
  /link /verbose:lib /SUBSYSTEM:CONSOLE,5.01 /MACHINE:X86 ^
  ws2_32.lib setupapi.lib advapi32.lib > link_verbose.txt

if exist aulon_xp.exe echo SUCCESS: aulon_xp.exe created.
