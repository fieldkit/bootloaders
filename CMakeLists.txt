project(bootloaders)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_MODULE_PATH $ENV{CMAKE_MODULE_PATH})

add_subdirectory(zero)
add_subdirectory(blink)
add_subdirectory(loader)
