
# Select the toolchain to compile with
CROSSCOMPILE=i686-elf-

CC      = $(CROSSCOMPILE)gcc
LD      = $(CROSSCOMPILE)ld
AS      = $(CROSSCOMPILE)as
OBJCOPY = $(CROSSCOMPILE)objcopy

export CC LD AS OBJCOPY

all: moose

moose: kernel.o
	$(OBJCOPY) -O binary $^ $@

kernel.o:
	$(MAKE) -f kernel/Makefile

qemu: all
	qemu-system-i386 -fda moose

.PHONY: all

