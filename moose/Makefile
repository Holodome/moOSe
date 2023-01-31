D := moose
OS_IMG := $(D)/moose.img
KERNEL_LINK := $(D)/arch/amd64/link.ld

KERNEL_OBJS := $(D)/arch/amd64/kstart.o $(D)/arch/amd64/long.o \
	$(D)/kstdio.o $(D)/kmem.o $(D)/arch/amd64/console_vga.o \
	$(D)/kmain.o $(D)/arch/amd64/memory_map.o $(D)/arch/amd64/isr.o \
	$(D)/arch/amd64/idt.o $(D)/arch/amd64/ata.o


$(OS_IMG): $(D)/arch/amd64/boot/boot.bin $(D)/kernel.bin
	cat $^ > $@

$(D)/kernel1.elf: $(KERNEL_OBJS) | $(KERNEL_LINK)
	$(LD) -T $(KERNEL_LINK) -o $@ $^

$(D)/kernel.bin: $(D)/kernel1.bin
	cp $^ $@
	truncate -s 512KB $@

include moose/arch/amd64/boot/Makefile
