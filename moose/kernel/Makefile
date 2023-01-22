D := moose/kernel
KERNEL := $(D)/kernel.elf

$(KERNEL): $(D)/boot.bin

BOOT_LDS := $(D)/arch/boot/link.ld
$(D)/boot.bin: $(D)/boot.o | $(BOOT_LDS)
	$(LD) --oformat=binary -T $(BOOT_LDS) -o $@ $^

$(D)/boot.o: $(D)/arch/boot/boot.s
	$(AS) -I$(D)/arch/boot $(ASFLAGS) -o $@ $^