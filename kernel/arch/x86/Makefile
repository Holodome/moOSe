include kernel/arch/x86/boot/Makefile

kernel/arch.o: kernel/arch/x86/boot.o
	$(LD) -T kernel/arch/x86/link.ld -o $@ $^
