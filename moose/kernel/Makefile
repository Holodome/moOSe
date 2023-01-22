D := moose/kernel
KERNEL := $(D)/kernel

$(KERNEL): $(D)/boot.bin $(D)/kernel.bin
	cat $^ > $@

$(D)/boot.bin: $(D)/boot.elf
	$(OBJCOPY) -O binary $^ $@

$(D)/boot.elf: $(D)/boot.o
	$(LD) -T $(D)/arch/boot/link.ld -o $@ $^

$(D)/boot.o: $(D)/arch/boot/boot.s
	$(AS) $(ASFLAGS) -o $@ $^

$(D)/kernel.bin: $(D)/kernel.elf
	$(OBJCOPY) -O binary $^ $@

$(D)/kernel.elf: $(D)/kernel-entry.o $(D)/kernel.o
	$(LD) -T $(D)/link.ld -o $@ $^

$(D)/kernel-entry.o: $(D)/arch/entry.s
	$(AS) $(ASFLAGS) -o $@ $^

$(D)/kernel.o: $(D)/kmain.c
	$(CC) $(CFLAGS) -o $@ $^
