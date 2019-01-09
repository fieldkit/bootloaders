include(Arduino)

enable_arduino_toolchain()

file(GLOB sources *.c *.cpp)

include_directories(../include)


# Small Bootloader
add_arduino_library(fk-bootloaders-samd21small "${sources}")

target_compile_options(fk-bootloaders-samd21small PUBLIC -DFK_BOOTLOADER_SMALL)

configure_firmware_linker_script(fk-bootloaders-samd21small "${CMAKE_CURRENT_SOURCE_DIR}/samd21x18_small_bootloader.ld")

add_arduino_bootloader(fk-bootloaders-samd21small)


# Self Flashing / Large Bootloader
add_arduino_library(fk-bootloaders-samd21large "${sources}")

find_package(Adafruit_NeoPixel)
target_link_libraries(fk-bootloaders-samd21large Adafruit_NeoPixel)

find_package(phylum)
target_link_libraries(fk-bootloaders-samd21large phylum)

find_package(arduino-logging)
target_link_libraries(fk-bootloaders-samd21large arduino-logging)

target_compile_options(fk-bootloaders-samd21large PUBLIC -DFK_BOOTLOADER_LARGE)
target_compile_options(fk-bootloaders-samd21large PUBLIC -DFK_BOOTLOADER_ENABLE_FLASH)
target_compile_options(fk-bootloaders-samd21large PUBLIC -DFK_BOOTLOADER_ENABLE_SERIAL5)

target_compile_options(arduino-logging PUBLIC -DARDUINO_LOGGING_DISABLE)

configure_firmware_linker_script(fk-bootloaders-samd21large "${CMAKE_CURRENT_SOURCE_DIR}/samd21x18_large_bootloader.ld")

add_arduino_bootloader(fk-bootloaders-samd21large)
