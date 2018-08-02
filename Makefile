#
# This Makefile calls the default, unmodified one in the bootloader directory.
#

all: build

gitdeps:
	simple-deps --config loader/arduino-libraries

loader/secrets.h:
	cp loader/secrets.h.template loader/secrets.h

build: gitdeps loader/secrets.h
	mkdir -p build
	cd build && cmake ../
	cd build && make

clean:
	rm -rf build

veryclean: clean

.PHONY: build
