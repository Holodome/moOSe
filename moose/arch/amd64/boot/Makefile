D := moose/arch/amd64/boot

STAGE1_OBJS := $(D)/boot.o
STAGE2_OBJS := $(D)/entry.o $(D)/print.o $(D)/a20.o $(D)/protected.o \
			   $(D)/memory.o $(D)/loader.o 
STAGE1_LINK := $(D)/stage1.ld.out
STAGE2_LINK := $(D)/stage2.ld.out

$(D)/stage1.elf: $(STAGE1_OBJS) | $(STAGE1_LINK)
	@echo "LD $@"
	$(Q)$(LD) $(LDFLAGS) -m elf_i386 -T $(STAGE1_LINK) -o $@ $^

$(D)/stage2.elf: $(STAGE2_OBJS) | $(STAGE2_LINK)
	@echo "LD $@"
	$(Q)$(LD) $(LDFLAGS) --gc-sections -m elf_i386 -T $(STAGE2_LINK) -o $@ $^

$(D)/%.o: $(D)/%.S
	@echo "AS $<"
	$(Q)$(CC) $(CFLAGS) -m32 -c -o $@ $<

$(D)/%.o: $(D)/%.c
	@echo "CC $<"
	$(Q)$(CC) $(filter-out -mcmodel=large, $(CFLAGS)) $(DEPFLAGS) -fdata-sections -ffunction-sections -Os -D__i686__ -m32 -c -o $@ $<

$(D)/%.o: $(D)/%.S
	@echo "CC $<"
	$(Q)$(CC) $(DEPFLAGS) -fdata-sections -ffunction-sections -Os -I. -D__i686__ -m32 -c -o $@ $<

