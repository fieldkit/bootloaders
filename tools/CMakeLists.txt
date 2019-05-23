include(Arduino)

enable_arduino_toolchain()

if(TARGET_M4)
  enable_m4()
endif()

if(TARGET_M0)
  enable_m0()
endif()

file(GLOB sources *.cpp)
add_arduino_library(fuses "fuses.cpp")

add_arduino_firmware(fuses)
