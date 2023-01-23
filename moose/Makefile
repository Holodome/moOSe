D := moose
OS_IMG := $(D)/moose.img

$(OS_IMG): $(D)/boot.bin $(D)/kernel.bin
	cat $^ > $@

$(D)/boot.elf: $(D)/arch/boot/boot.o
	$(LD) -T $(D)/arch/boot/link.ld -o $@ $^

$(D)/kernel.elf: $(D)/arch/entry.o $(D)/arch/ports.o $(D)/kmain.o $(D)/vga.o
	$(LD) -T $(D)/link.ld -o $@ $^

