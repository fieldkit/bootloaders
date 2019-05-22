include(Arduino)

enable_arduino_toolchain()

file(GLOB sources *.cpp)
add_arduino_library(fuses "fuses.cpp")

add_arduino_firmware(fuses)
