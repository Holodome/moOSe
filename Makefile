# Select the toolchain to compile with
CROSSCOMPILE=i686-elf-

CC      = $(CROSSCOMPILE)gcc
LD      = $(CROSSCOMPILE)ld
AS      = $(CROSSCOMPILE)as
OBJCOPY = $(CROSSCOMPILE)objcopy

export CC LD AS OBJCOPY

ASFLAGS = -msyntax=att --warn --fatal-warnings

export ASFLAGS 

all: moose.img

moose.img: moose.elf
	$(OBJCOPY) -O binary $< $@

moose.elf: kernel/kernel.o
	$(LD) -o $@ -T kernel/link.x --build-id=none $^

include kernel/Makefile

qemu: all
	qemu-system-i386 -fda moose.img

clean: 
	rm -f $(shell find . -name "*.o" -o -name "*.elf" -o -name "*.img")

.PHONY: all clean

