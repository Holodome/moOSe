include moose/arch/amd64/boot/Makefile

D := moose
OS_IMG := $(D)/moose.img

KERNEL_LINK := $(D)/arch/amd64/kernel.ld.out
KERNEL_OBJS := $(D)/arch/amd64/kstart.o $(D)/kstdio.o $(D)/arch/amd64/vga.o \
	$(D)/kmain.o $(D)/arch/amd64/memory_map.o $(D)/arch/amd64/isr.o \
	$(D)/arch/amd64/idt.o $(D)/arch/amd64/ata.o $(D)/fs/fat.o \
	$(D)/arch/amd64/keyboard.o $(D)/disk.o $(D)/physmem.o \
	$(D)/disk.o $(D)/arch/amd64/rtc.o $(D)/tty.o \
	$(D)/kmalloc.o $(D)/device.o $(D)/errno.o $(D)/arch/amd64/virtmem.o \
	$(D)/vmalloc.o $(D)/arch/amd64/cpu.o $(D)/panic.o \
	$(D)/kthread.o $(D)/shell.o $(D)/arch/amd64/asm.o

KERNEL_BOOT_OBJS := $(D)/arch/amd64/kboot.o $(D)/arch/amd64/long.o
KERNEL_BOOT_LINK := $(D)/arch/amd64/kernel-boot.ld.out

$(OS_IMG): $(D)/arch/amd64/boot/stage1.bin $(D)/arch/amd64/boot/stage2.bin \
	$(D)/kernel.img
	$(Q)./scripts/generate_mbr_image.py $^ $@

$(D)/kernel1.elf: $(KERNEL_OBJS) | $(KERNEL_LINK)
	@echo "LD $@"
	$(Q)$(LD) $(LDFLAGS) -T $(KERNEL_LINK) -o $@ $^

$(D)/kernel-boot.elf: $(KERNEL_BOOT_OBJS) | $(KERNEL_BOOT_LINK)
	@echo "LD $@"
	$(Q)$(LD) $(LDFLAGS) -T $(KERNEL_BOOT_LINK) -o $@ $^

$(D)/kernel.img: $(D)/kernel-boot.bin $(D)/kernel1.bin
	$(Q)cat $^ > $(D)/kernel.bin
	$(Q)dd if=/dev/zero of=$@ bs=1K count=720 status=none
	$(Q)mformat -i $@ -f 720
	$(Q)mcopy -i $@ $(D)/kernel.bin ::

