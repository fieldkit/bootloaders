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

build: gitdeps loader/secrets.h
	mkdir -p $(BUILD)
	cd $(BUILD) && cmake ../
	$(MAKE) -C $(BUILD)

clean:
	rm -rf $(BUILD)

veryclean: clean
	rm -rf gitdeps

.PHONY: build
