set(ARDUINO_IDE "${CMAKE_CURRENT_SOURCE_DIR}/../../arduino-1.8.3") 
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../../arduino-cmake-minimal/cmake/")

include(ArduinoMinimal)

set(ARDUINO_INCLUDE_CORE OFF)

set(LIBRARIES)
file(GLOB SOURCE_FILES *.c *.cpp)

include_directories(./)
include_directories(../include)

set(ARDUINO_BOOTLOADER "${CMAKE_CURRENT_SOURCE_DIR}/samd21x18_small_bootloader.ld")

arduino(fk-bootloaders-samd21small "${SOURCE_FILES}" "${LIBRARIES}")
target_compile_options(fk-bootloaders-samd21small PUBLIC -DFK_BOOTLOADER_SMALL)

set(ARDUINO_BOOTLOADER "${CMAKE_CURRENT_SOURCE_DIR}/samd21x18_large_bootloader.ld")

arduino(fk-bootloaders-samd21large "${SOURCE_FILES}" "${LIBRARIES}")
target_compile_options(fk-bootloaders-samd21large PUBLIC -DFK_BOOTLOADER_LARGE)
target_compile_options(fk-bootloaders-samd21large PUBLIC -DFK_BOOTLOADER_ENABLE_FLASH)
target_compile_options(fk-bootloaders-samd21large PUBLIC -DFK_BOOTLOADER_ENABLE_SERIAL5)
target_compile_options(arduino_zero_phylum PUBLIC -DPHYLUM_DISABLE_LOGGING)
