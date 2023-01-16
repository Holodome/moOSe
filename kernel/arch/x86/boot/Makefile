kernel/arch/x86/boot.o: kernel/arch/x86/boot/stage1.o kernel/arch/x86/boot/stage2.o
	$(LD) -relocatable -o $@ $^

kernel/arch/x86/boot/stage1.o: kernel/arch/x86/boot/stage1.s
	$(AS) $(ASFLAGS) -o $@ $^