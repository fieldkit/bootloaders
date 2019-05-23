#
# This Makefile calls the default, unmodified one in the bootloader directory.
#

BUILD ?= $(abspath build)

all: build

gitdeps:
	simple-deps --config loader/dependencies.sd
	simple-deps --config zero/dependencies.sd

loader/secrets.h:
	cp loader/secrets.h.template loader/secrets.h

build: m0 m4

dependencies: gitdeps loader/secrets.h

m0: dependencies
	mkdir -p build/m0
	cd build/m0 && cmake -D TARGET_M0=ON ../../
	cd build/m0 && $(MAKE)

m4: dependencies
	mkdir -p build/m4
	cd build/m4 && cmake -D TARGET_M4=ON ../../
	cd build/m4 && $(MAKE)

clean:
	rm -rf $(BUILD)

veryclean: clean
	rm -rf gitdeps

.PHONY: build
