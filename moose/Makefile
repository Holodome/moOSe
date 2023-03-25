include moose/arch/amd64/boot/Makefile

D := moose
OS_IMG := $(D)/moose.img

KERNEL_LINK := $(D)/arch/amd64/kernel.ld.out
KERNEL_OBJS := $(D)/arch/amd64/kstart.o $(D)/kstdio.o $(D)/drivers/vga.o \
	$(D)/arch/amd64/kmain.o $(D)/arch/amd64/memmap.o $(D)/arch/amd64/isr.o \
	$(D)/arch/amd64/idt.o $(D)/drivers/ata.o $(D)/fs/fat.o \
	$(D)/drivers/keyboard.o $(D)/drivers/disk.o $(D)/mm/physmem.o \
	$(D)/arch/amd64/rtc.o $(D)/drivers/tty.o $(D)/sched/spinlock.o \
	$(D)/mm/kmalloc.o $(D)/blk_device.o $(D)/arch/amd64/virtmem.o \
	$(D)/mm/vmalloc.o $(D)/arch/amd64/cpu.o $(D)/panic.o \
	$(D)/kthread.o $(D)/shell.o $(D)/arch/amd64/asm.o $(D)/idle.o \
	$(D)/mm/slab.o $(D)/string.o $(D)/ctype.o $(D)/fs/ext2.o \
	$(D)/fs/vfs.o $(D)/arch/refcount.o $(D)/fs/ramfs.o

KERNEL_BOOT_OBJS := $(D)/arch/amd64/kboot.o $(D)/arch/amd64/long.o
KERNEL_BOOT_LINK := $(D)/arch/amd64/kernel-boot.ld.out

$(OS_IMG): $(D)/arch/amd64/boot/stage1.bin $(D)/arch/amd64/boot/stage2.bin \
	$(D)/kernel.img 
	$(Q)./scripts/generate_mbr_image.py $^ $@

scripts/ext2.img: 

$(D)/kernel1.elf: $(KERNEL_OBJS) | $(KERNEL_LINK)
	@echo "LD $@"
	$(Q)$(LD) $(LDFLAGS) -T $(KERNEL_LINK) -o $@ $^

$(D)/kernel-boot.elf: $(KERNEL_BOOT_OBJS) | $(KERNEL_BOOT_LINK)
	@echo "LD $@"
	$(Q)$(LD) $(LDFLAGS) -T $(KERNEL_BOOT_LINK) -o $@ $^

$(D)/kernel.img: $(D)/kernel-boot.bin $(D)/kernel1.bin
	$(Q)cat $^ > $(D)/kernel.bin
	$(Q)dd if=/dev/zero of=$@ bs=1K count=720 status=none > /dev/null
	$(Q)mformat -i $@ -f 720
	$(Q)mcopy -i $@ $(D)/kernel.bin ::

