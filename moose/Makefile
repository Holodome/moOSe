D := moose
OS_IMG := $(D)/moose.img
KERNEL_LINK := $(D)/arch/amd64/link.ld

$(OS_IMG): $(D)/arch/amd64/boot/boot.bin $(D)/kernel.bin
	cat $^ > $@

$(D)/kernel.elf: $(D)/arch/amd64/kstart.o $(D)/arch/amd64/long.o \
	$(D)/kstdio.o $(D)/kmem.o $(D)/arch/amd64/console_vga.o
	$(LD) -T $(KERNEL_LINK) -o $@ $^

include moose/arch/amd64/boot/Makefile
