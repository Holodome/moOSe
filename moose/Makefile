D := moose
OS_IMG_DEPS := $(D)/arch/amd64/boot/boot.bin
OS_IMG := $(D)/moose.img

$(OS_IMG): $(OS_IMG_DEPS)
	cp $^ $@

include moose/arch/amd64/boot/Makefile
