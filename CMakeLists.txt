cmake_minimum_required(VERSION 3.16)

project(iQueBrewTicketModifier VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(IQUEBREW_BUILD_GUI "Build the Windows GUI" ${WIN32})
option(IQUEBREW_ENABLE_WRITING "Enable NAND/file write commands" OFF)
option(IQUEBREW_ENABLE_USB_LOGGING "Enable USB transfer logging" OFF)

find_package(unofficial-libusb CONFIG QUIET)
if(TARGET unofficial::libusb::libusb)
  add_library(libusb-1.0::libusb INTERFACE IMPORTED)
  target_link_libraries(libusb-1.0::libusb INTERFACE unofficial::libusb::libusb)
endif()

if(NOT TARGET libusb-1.0::libusb)
  find_package(PkgConfig QUIET)
  if(PkgConfig_FOUND)
    pkg_check_modules(LIBUSB QUIET libusb-1.0)
  endif()
endif()

if(TARGET libusb-1.0::libusb)
elseif(LIBUSB_FOUND)
  add_library(libusb-1.0::libusb INTERFACE IMPORTED)
  target_include_directories(libusb-1.0::libusb INTERFACE ${LIBUSB_INCLUDE_DIRS})
  target_link_libraries(libusb-1.0::libusb INTERFACE ${LIBUSB_LIBRARIES})
  target_link_directories(libusb-1.0::libusb INTERFACE ${LIBUSB_LIBRARY_DIRS})
  target_compile_options(libusb-1.0::libusb INTERFACE ${LIBUSB_CFLAGS_OTHER})
else()
  find_path(LIBUSB_INCLUDE_DIR
    NAMES libusb.h
    PATH_SUFFIXES libusb-1.0 libusb
  )
  find_library(LIBUSB_LIBRARY
    NAMES usb-1.0 libusb-1.0 libusb
  )
  if(LIBUSB_INCLUDE_DIR AND LIBUSB_LIBRARY)
    add_library(libusb-1.0::libusb UNKNOWN IMPORTED)
    set_target_properties(libusb-1.0::libusb PROPERTIES
      IMPORTED_LOCATION "${LIBUSB_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${LIBUSB_INCLUDE_DIR}"
    )
  elseif(UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    set(LIBUSB_CONFIG_DIR "${CMAKE_CURRENT_BINARY_DIR}/libusb_config")
    file(MAKE_DIRECTORY "${LIBUSB_CONFIG_DIR}")
    file(WRITE "${LIBUSB_CONFIG_DIR}/config.h" "
#define DEFAULT_VISIBILITY __attribute__((visibility(\"default\")))
#define ENABLE_LOGGING 1
#define HAVE_ASM_TYPES_H 1
#define HAVE_DECL_TFD_CLOEXEC 1
#define HAVE_DECL_TFD_NONBLOCK 1
#define HAVE_DLFCN_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_LINUX_NETLINK_H 1
#define HAVE_MEMORY_H 1
#define HAVE_PIPE2 1
#define HAVE_POLL_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_SYSLOG_H 1
#define HAVE_SYS_SOCKET_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_UNISTD_H 1
#define OS_LINUX 1
#define POLL_NFDS_TYPE nfds_t
#define THREADS_POSIX 1
#define _GNU_SOURCE 1
")

    add_library(bundled_libusb STATIC
      libusb-1.0.23/libusb/core.c
      libusb-1.0.23/libusb/descriptor.c
      libusb-1.0.23/libusb/hotplug.c
      libusb-1.0.23/libusb/io.c
      libusb-1.0.23/libusb/strerror.c
      libusb-1.0.23/libusb/sync.c
      libusb-1.0.23/libusb/os/linux_usbfs.c
      libusb-1.0.23/libusb/os/linux_netlink.c
      libusb-1.0.23/libusb/os/poll_posix.c
      libusb-1.0.23/libusb/os/threads_posix.c
    )
    target_include_directories(bundled_libusb PUBLIC libusb-1.0.23/libusb)
    target_include_directories(bundled_libusb PRIVATE "${LIBUSB_CONFIG_DIR}")
    target_compile_definitions(bundled_libusb PRIVATE _GNU_SOURCE)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
      target_compile_options(bundled_libusb PRIVATE -Wno-format-zero-length)
    endif()
    target_link_libraries(bundled_libusb PUBLIC Threads::Threads)
    add_library(libusb-1.0::libusb ALIAS bundled_libusb)
  else()
    message(FATAL_ERROR "libusb-1.0 was not found. Install libusb-1.0 development files or set LIBUSB_INCLUDE_DIR and LIBUSB_LIBRARY.")
  endif()
endif()

set(AULON_CORE_SOURCES
  src/commands.c
  src/fs.c
  src/aulon_io.c
  src/menu.c
  src/menu_func.c
  src/player_comms.c
  src/usb.c
  src/usb_log.c
  src/server.c
)

add_library(aulon_core STATIC ${AULON_CORE_SOURCES})
target_include_directories(aulon_core PUBLIC src)
target_link_libraries(aulon_core PUBLIC libusb-1.0::libusb)
target_compile_definitions(aulon_core PUBLIC
  AULON_WRITING_ENABLED=$<IF:$<BOOL:${IQUEBREW_ENABLE_WRITING}>,1,0>
  AULON_LOGGING_ENABLED=$<IF:$<BOOL:${IQUEBREW_ENABLE_USB_LOGGING}>,1,0>
)

if(MSVC)
  target_compile_definitions(aulon_core PUBLIC _CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN)
  target_compile_options(aulon_core PRIVATE /W4)
else()
  target_compile_options(aulon_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32)
  target_link_libraries(aulon_core PUBLIC ws2_32 setupapi advapi32)
endif()

add_executable(aulon src/main.c)
target_link_libraries(aulon PRIVATE aulon_core)

if(WIN32 AND IQUEBREW_BUILD_GUI)
  enable_language(RC)
  add_executable(ique_modern WIN32
    gui/modern_gui.cpp
    gui/resource.rc
  )
  target_include_directories(ique_modern PRIVATE gui)
  target_compile_definitions(ique_modern PRIVATE GUI_BUILD WIN32_LEAN_AND_MEAN NOMINMAX)
  target_link_libraries(ique_modern PRIVATE aulon_core gdiplus comctl32 comdlg32 shlwapi user32 gdi32 shell32)
endif()
