include(Arduino)

enable_arduino_toolchain()

include_directories(./)
include_directories(../include)

file(GLOB sources *.cpp)
add_arduino_library(loader "${sources}")

configure_firmware_linker_script(loader "${CMAKE_CURRENT_SOURCE_DIR}/samd21x18_large_bootloader.ld")

find_package(phylum)
target_link_libraries(loader phylum)
target_compile_options(phylum PUBLIC -DPHYLUM_ENABLE_SERIAL_FLASH)

find_package(WiFi101)
target_link_libraries(loader WiFi101)

add_arduino_firmware(loader)
