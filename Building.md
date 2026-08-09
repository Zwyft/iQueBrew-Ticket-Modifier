# Building iQueBrew on Modern Systems

The supported build path is CMake. The old batch files under `scripts/` are legacy Windows XP-oriented helpers and are not required for normal builds.

## Dependencies

Install a C/C++ compiler and CMake 3.16 or newer. System libusb 1.0 development files are preferred; on Linux, the build can fall back to the vendored libusb source if they are not installed.

Linux examples:

```sh
sudo apt install build-essential cmake pkg-config libusb-1.0-0-dev
```

```sh
sudo dnf install gcc gcc-c++ cmake pkgconf-pkg-config libusb1-devel
```

macOS example:

```sh
brew install cmake libusb pkg-config
```

Windows options:

- Visual Studio 2022 with the Desktop development with C++ workload.
- libusb installed through vcpkg, MSYS2, or a local libusb SDK.
- A WinUSB-compatible driver for the iQue Player, commonly installed with Zadig.

## Build the CLI

```sh
cmake -S . -B build
cmake --build build
```

The CLI executable is `build/aulon` on Unix-like systems or `build\Debug\aulon.exe` / `build\Release\aulon.exe` with Visual Studio generators.

## Build on Windows with vcpkg

```powershell
vcpkg install libusb:x64-windows
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
```

The Windows GUI is built by default on Windows. To disable it:

```powershell
cmake -S . -B build -DIQUEBREW_BUILD_GUI=OFF
```

## Optional Features

Write commands are disabled by default. Enable them only when you intend to perform NAND/file write operations:

```sh
cmake -S . -B build -DIQUEBREW_ENABLE_WRITING=ON
```

USB transfer logging is disabled by default because it can produce very large logs:

```sh
cmake -S . -B build -DIQUEBREW_ENABLE_USB_LOGGING=ON
```

## Running

Connect the iQue Player, make sure the operating system can access the USB device, then run:

```sh
./build/aulon
```

On Linux, create a udev rule or run with sufficient permissions if libusb cannot open the device. On Windows, the device must use a libusb-compatible driver such as WinUSB.
