D := moose/arch/amd64/boot

STAGE1_OBJS := $(D)/boot.o
STAGE2_OBJS := $(D)/entry.o $(D)/print.o $(D)/a20.o $(D)/protected.o \
			   $(D)/memory.o $(D)/loader.o $(D)/arith64.o
STAGE1_LINK := $(D)/stage1.ld
STAGE2_LINK := $(D)/stage2.ld

$(D)/stage1.elf: $(STAGE1_OBJS) | $(STAGE1_LINK)
	@echo "LD $@"
	$(Q)$(LD) -m elf_i386 -T $(STAGE1_LINK) -o $@ $^

$(D)/stage2.elf: $(STAGE2_OBJS) | $(STAGE2_LINK)
	@echo "LD $@"
	$(Q)$(LD) -m elf_i386 -T $(STAGE2_LINK) -o $@ $^

$(D)/%.o: $(D)/%.S
	@echo "AS $^"
	$(Q)$(CC) $(CFLAGS) -m32 -c -o $@ $<

$(D)/%.o: $(D)/%.c
	@echo "CC $^"
	$(Q)$(CC) $(CFLAGS) $(DEPFLAGS) -Os -D__i686__ -m32 -c -o $@ $<

