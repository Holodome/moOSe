D := moose
OS_IMG := $(D)/moose.img
KERNEL_LINK := $(D)/arch/amd64/link.ld

KERNEL_OBJS := $(D)/arch/amd64/kstart.o $(D)/arch/amd64/long.o \
	$(D)/kstdio.o $(D)/kmem.o $(D)/arch/amd64/tty_vga.o \
	$(D)/kmain.o $(D)/arch/amd64/memory_map.o $(D)/arch/amd64/isr.o \
	$(D)/arch/amd64/idt.o $(D)/arch/amd64/ata.o $(D)/fs/fat.o \
	$(D)/arch/amd64/tty_keyboard.o $(D)/disk.o $(D)/arch/amd64/physmem.o \
	$(D)/disk.o $(D)/arch/amd64/rtc.o $(D)/arch/amd64/processor.o \
	$(D)/kmalloc.o $(D)/device.o $(D)/errno.o

$(OS_IMG): $(D)/arch/amd64/boot/stage1.bin $(D)/arch/amd64/boot/stage2.bin \
	$(D)/kernel.bin
	$(Q)./scripts/generate_mbr_image.py $^ $@

$(D)/kernel1.elf: $(KERNEL_OBJS) | $(KERNEL_LINK)
	@echo "LD $@"
	$(Q)$(LD) -T $(KERNEL_LINK) -o $@ $^

$(D)/kernel.bin: $(D)/kernel1.bin
	$(Q)dd if=/dev/zero of=$@ bs=1K count=720 status=none
	$(Q)mformat -i $@ -f 720
	$(Q)mcopy -i $@ $^ ::

include moose/arch/amd64/boot/Makefile
