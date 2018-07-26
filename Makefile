#
# This Makefile calls the default, unmodified one in the bootloader directory.
#

all: build

gitdeps:
	simple-deps --config loader/arduino-libraries

build: gitdeps
	mkdir -p build
	cd build && cmake ../
	cd build && make
	cd zero && MODULE_PATH=~/conservify/arduino-1.8.3/packages/arduino BOARD_ID=arduino_zero NAME=samd21_sam_ba make

clean:
	rm -rf build
	cd zero && MODULE_PATH=~/conservify/arduino-1.8.3/packages/arduino BOARD_ID=arduino_zero NAME=samd21_sam_ba make clean

.PHONY: build
