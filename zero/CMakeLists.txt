include(Arduino)

enable_arduino_toolchain()

find_package(phylum)
find_package(arduino-logging)

file(GLOB sources *.c *.cpp)

include_directories(../include)


# Small Bootloader
add_arduino_library(fk-bootloaders-samd21small "${sources}")

target_compile_options(fk-bootloaders-samd21small PUBLIC -DFK_BOOTLOADER_SMALL)

configure_firmware_linker_script(fk-bootloaders-samd21small "${CMAKE_CURRENT_SOURCE_DIR}/samd21x18_small_bootloader.ld")

add_arduino_bootloader(fk-bootloaders-samd21small)


# Self Flashing / Large Bootloader
add_arduino_library(fk-bootloaders-samd21large "${sources}")

target_compile_options(fk-bootloaders-samd21large PUBLIC -DFK_BOOTLOADER_LARGE)
target_compile_options(fk-bootloaders-samd21large PUBLIC -DFK_BOOTLOADER_ENABLE_FLASH)
target_compile_options(fk-bootloaders-samd21large PUBLIC -DFK_BOOTLOADER_ENABLE_SERIAL5)

target_compile_options(arduino-logging PUBLIC -DARDUINO_LOGGING_DISABLE)

target_link_libraries(fk-bootloaders-samd21large phylum)
target_link_libraries(fk-bootloaders-samd21large arduino-logging)

configure_firmware_linker_script(fk-bootloaders-samd21large "${CMAKE_CURRENT_SOURCE_DIR}/samd21x18_large_bootloader.ld")

add_arduino_bootloader(fk-bootloaders-samd21large)
