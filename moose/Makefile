D := moose
OS_IMG := $(D)/moose.img
KERNEL_LINK := $(D)/arch/amd64/link.ld

KERNEL_OBJS := $(D)/arch/amd64/kstart.o $(D)/arch/amd64/long.o \
	$(D)/kstdio.o $(D)/kmem.o $(D)/arch/amd64/tty_vga.o \
	$(D)/kmain.o $(D)/arch/amd64/memory_map.o $(D)/arch/amd64/isr.o \
	$(D)/arch/amd64/idt.o $(D)/arch/amd64/ata.o $(D)/fs/fat.o \
	$(D)/arch/amd64/tty_keyboard.o $(D)/disk.o

$(OS_IMG): $(D)/arch/amd64/boot/stage1.bin $(D)/arch/amd64/boot/stage2.bin \
	$(D)/kernel.bin
	./scripts/generate_mbr_image.py $^ $@

$(D)/kernel.elf: $(KERNEL_OBJS) | $(KERNEL_LINK)
	$(LD) -T $(KERNEL_LINK) -o $@ $^

include moose/arch/amd64/boot/Makefile
