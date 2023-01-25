D := moose
OS_IMG := $(D)/moose.img

$(OS_IMG): $(D)/boot.bin $(D)/kernel.bin
	cat $^ > $@

$(D)/boot.elf: $(D)/arch/boot/boot.o
	$(LD) -T $(D)/arch/boot/link.ld -o $@ $^

$(D)/kernel.elf: $(D)/arch/entry.o $(D)/kmain.o $(D)/vga.o $(D)/kprint.o $(D)/interrupt.o $(D)/isr.o $(D)/keyboard.o $(D)/arch/print.o
	$(LD) -T $(D)/link.ld -o $@ $^

