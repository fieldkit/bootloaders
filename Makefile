#
# This Makefile calls the default, unmodified one in the bootloader directory.
#

build:
	cd zero && MODULE_PATH=~/conservify/arduino-packages/packages/arduino BOARD_ID=arduino_zero NAME=samd21_sam_ba make

clean:
	cd zero && MODULE_PATH=~/conservify/arduino-packages/packages/arduino BOARD_ID=arduino_zero NAME=samd21_sam_ba make clean
