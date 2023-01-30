D := moose/arch/amd64/boot

STAGE1_OBJS := $(D)/boot.o
STAGE2_OBJS := $(D)/entry.o $(D)/print.o $(D)/a20.o $(D)/ata.o
STAGE1_LINK := $(D)/stage1.ld
STAGE2_LINK := $(D)/stage2.ld

$(D)/boot.bin: $(D)/stage1.bin $(D)/stage2.bin
	cat $^ > $@

$(D)/stage1.elf: $(STAGE1_OBJS) | $(STAGE1_LINK)
	$(LD) -T $(STAGE1_LINK) -o $@ $^

$(D)/stage2.elf: $(STAGE2_OBJS) | $(STAGE2_LINK)
	$(LD) -T $(STAGE2_LINK) -o $@ $^


