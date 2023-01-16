include kernel/arch/x86/Makefile

kernel/kernel.o: kernel/arch.o
	$(LD) -T kernel/link.ld -o $@ $^
