include kernel/arch/x86/boot/Makefile

kernel/arch.o: kernel/arch/x86/boot.o
	$(LD) -relocatable -o $@ $^
