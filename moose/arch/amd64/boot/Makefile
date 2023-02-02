D := moose/arch/amd64/boot

STAGE1_OBJS := $(D)/boot.o
STAGE2_OBJS := $(D)/entry.o $(D)/print.o $(D)/a20.o $(D)/protected.o \
			   $(D)/memory.o $(D)/loader.o
STAGE1_LINK := $(D)/stage1.ld
STAGE2_LINK := $(D)/stage2.ld

$(D)/boot.bin: $(D)/stage1.bin $(D)/stage2.bin
	cat $^ > $@

$(D)/stage1.elf: $(STAGE1_OBJS) | $(STAGE1_LINK)
	$(LD) -m elf_i386 -T $(STAGE1_LINK) -o $@ $^

$(D)/stage2.bin: $(D)/stage2_1.bin
	cp $^ $@
	truncate -s 20K $@

$(D)/stage2_1.elf: $(STAGE2_OBJS) | $(STAGE2_LINK)
	$(LD) -m elf_i386 -T $(STAGE2_LINK) -o $@ $^

$(D)/loader.o: $(D)/loader.c moose/fs/fat.c moose/disk.c moose/arch/amd64/ata.c
	$(CC) $(CFLAGS) -D__i686__ -m32 -c -o $@ $<

$(D)/%.o: $(D)/%.S
	$(CC) $(CFLAGS) -m32 -c -o $@ $<

