D := moose/arch/amd64/boot

STAGE1_OBJS := $(D)/boot.o
STAGE2_OBJS := $(D)/entry.o $(D)/print.o $(D)/a20.o $(D)/ata.o $(D)/protected.o \
			   $(D)/memory.o $(D)/loader.o
STAGE1_LINK := $(D)/stage1.ld
STAGE2_LINK := $(D)/stage2.ld

$(D)/boot.bin: $(D)/stage1.bin $(D)/stage2.bin
	cat $^ > $@

$(D)/stage1.elf: $(STAGE1_OBJS) | $(STAGE1_LINK)
	$(LD) -T $(STAGE1_LINK) -o $@ $^

$(D)/stage2.elf: $(STAGE2_OBJS) | $(STAGE2_LINK)
	$(LD) -O elf32-i386 -T $(STAGE2_LINK) -o $@ $^

$(D)/loader.o: $(D)/loader.c moose/fs/fat.c moose/disk.c moose/arch/amd64/ata.c
	$(CC) $(CFLAGS) -Wl,-Oelf32-i386 -c -o $@ $<

