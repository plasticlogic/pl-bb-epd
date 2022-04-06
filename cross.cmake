set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(cc_toolchain_path C:/SysGCC/beaglebone)
#set(sysroot_target ${cc_toolchain_path}/arm-linux-gnueabihf/sysroot)
set(sysroot_target C:\\Users\\lars.goerner\\Desktop\\sysroot)
set(tools ${cc_toolchain_path}/bin)

set(CMAKE_C_COMPILER ${tools}/arm-linux-gnueabihf-gcc.exe)
set(CMAKE_CXX_COMPILER ${tools}/arm-linux-gnueabihf-g++.exe)
set(CMAKE_SYSROOT ${sysroot_target})

set(CMAKE_CXX_FLAGS " -march=armv7-a -marm -mfpu=neon -mfloat-abi=hard --sysroot=${sysroot_target}")
set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_EXE_LINKER_FLAGS "--sysroot=${sysroot_target}")

set(CMAKE_FIND_ROOT_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_MODE_PACKAGE ONLY)