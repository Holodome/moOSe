# Select the toolchain to compile with
CROSSCOMPILE = i686-elf-

CC      := $(CROSSCOMPILE)gcc
LD      := $(CROSSCOMPILE)ld
AS      := $(CROSSCOMPILE)as
OBJCOPY := $(CROSSCOMPILE)objcopy

export CC LD AS OBJCOPY

ASFLAGS := -msyntax=att --warn --fatal-warnings
CFLAGS  := -Wall -Werror -Wextra -Wpedantic -std=c99 -ffreestanding -nostdlib -nostartfiles -Wl,-r

export ASFLAGS CFLAGS

TARGET_IMG := moose.img

include moose/Makefile

all: $(TARGET_IMG)

$(TARGET_IMG): $(OS_IMG)
	cp $< $@

qemu: all
	qemu-system-i386 -fda moose.img

clean:
	rm $(shell find . -name "*.o" -o -name "*.d" -o -name "*.bin" -o -name "*.img")
