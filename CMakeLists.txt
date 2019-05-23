project(bootloaders)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_MODULE_PATH $ENV{CMAKE_MODULE_PATH})

option(TARGET_M0 "Build M0 binaries" OFF)
option(TARGET_M4 "Build M4 binaries" OFF)

add_subdirectory(zero)
add_subdirectory(blink)
add_subdirectory(loader)
add_subdirectory(tools)
