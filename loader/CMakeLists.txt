set(ARDUINO_IDE "${CMAKE_CURRENT_SOURCE_DIR}/../../arduino-1.8.3") 
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../../arduino-cmake-minimal/cmake/")

include(ArduinoMinimal)

set(ARDUINO_BOOTLOADER "${CMAKE_CURRENT_SOURCE_DIR}/samd21x18_large_bootloader.ld")

set(SOURCE_FILES main.cpp)
set(LIBRARIES SPI Wire)

include_directories(./)

arduino(loader "${SOURCE_FILES}" "${LIBRARIES}")
