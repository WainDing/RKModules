cmake_minimum_required(VERSION 2.8)

option(ARMLINUX "arm linux option" OFF)
option(AARCHLINUX "aarch linux option OFF")

if (ARMLINUX)
set(ARCH "arm")
set(CMAKE_C_COMPILER "arm-linux-gnueabihf-gcc")
set(CMAKE_CXX_COMPILER "arm-linux-gnueabihf-g++")
endif()

if(AARCHLINUX)
set(ARCH "aarch")
set(CMAKE_C_COMPILER "/home/tiger/rk3399_linux/custorm/prebuilts/gcc/linux-x86/aarch64/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "/home/tiger/rk3399_linux/custorm/prebuilts/gcc/linux-x86/aarch64/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++")
endif()

project(LibRKMedia)

set(CMAKE_C_STANGARD 99)
set(CMAKE_CXX_STANGARD 11)

add_subdirectory(src)
add_subdirectory(demo)