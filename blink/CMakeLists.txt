include(Arduino)

enable_arduino_toolchain()

include_directories(./)

add_arduino_library(blink main.cpp)

add_arduino_firmware(blink)
