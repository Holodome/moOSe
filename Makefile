
# Select the toolchain to compile with
CROSSCOMPILE=i686-elf-

CC      = $(CROSSCOMPILE)gcc
LD      = $(CROSSCOMPILE)ld
AS      = $(CROSSCOMPILE)as
OBJCOPY = $(CROSSCOMPILE)objcopy

export CC LD AS OBJCOPY

moose.img: kernel.o

kernel.o:
	$(MAKE) -f kernel/Makefile

