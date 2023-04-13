include moose/arch/amd64/boot/Makefile

D := moose
OS_IMG := $(D)/moose.img

KERNEL_LINK := $(D)/arch/amd64/kernel.ld.out

KERNEL_OBJS := \
	$(D)/arch/amd64/kstart.o \
	$(D)/arch/amd64/kmain.o \
	$(D)/arch/amd64/memmap.o \
	$(D)/arch/amd64/idt.o \
	$(D)/arch/amd64/rtc.o \
	$(D)/arch/amd64/virtmem.o \
	$(D)/arch/amd64/cpu.o \
	$(D)/arch/amd64/delay.o \
	$(D)/arch/refcount.o \
	$(D)/drivers/ata.o \
	$(D)/drivers/disk.o \
	$(D)/drivers/rtl8139.o \
	$(D)/drivers/pci.o \
	$(D)/mm/resource.o \
	$(D)/mm/physmem.o \
	$(D)/mm/kmalloc.o \
	$(D)/mm/slab.o \
	$(D)/sched/locks.o \
	$(D)/sched/process.o \
	$(D)/sched/mutex.o \
	$(D)/fs/ext2.o \
	$(D)/fs/vfs.o \
	$(D)/fs/fat.o \
	$(D)/fs/ramfs.o \
	$(D)/net/ip.o \
	$(D)/net/arp.o \
	$(D)/net/inet.o \
	$(D)/net/eth.o \
	$(D)/net/netdaemon.o \
	$(D)/net/icmp.o \
	$(D)/net/udp.o \
	$(D)/net/frame.o \
	$(D)/net/interface.o \
	$(D)/tty/console.o \
	$(D)/tty/vga_console.o \
	$(D)/tty/vterm.o \
	$(D)/blk_device.o \
	$(D)/panic.o \
	$(D)/idle.o \
	$(D)/string.o \
	$(D)/ctype.o \
	$(D)/kstdio.o \
	$(D)/time.o \

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
	$(Q)cat $^ > $@

