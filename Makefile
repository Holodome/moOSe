# Select the toolchain to compile with
CROSSCOMPILE = i686-elf-

CC      := $(CROSSCOMPILE)gcc
LD      := $(CROSSCOMPILE)ld
AS      := $(CROSSCOMPILE)as
OBJCOPY := $(CROSSCOMPILE)objcopy
QEMU    := qemu-system-i386
GDB     := i386-elf-gdb

export CC LD AS OBJCOPY

ASFLAGS = -msyntax=att --warn --fatal-warnings
CFLAGS  = -Wall -Werror -Wextra -Wpedantic -std=c99 -ffreestanding -nostdlib -nostartfiles -Wl,-r

ifneq ($(DEBUG),)
	ASFLAGS += -ggdb
	CFLAGS += -ggdb
endif

export ASFLAGS CFLAGS

TARGET_IMG := moose.img

include moose/Makefile

all: $(TARGET_IMG)

$(TARGET_IMG): $(OS_IMG)
	cp $< $@

qemu: all
	$(QEMU) -fda moose.img

qemu-debug: all
	$(QEMU) -s -S -fda moose.img -d guest_errors,int & \
  	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file moose/kernel/kernel.elf"

clean:
	rm $(shell find . -name "*.o" \
		-o -name "*.d" \
		-o -name "*.bin" \
		-o -name "*.img")

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.s
	$(AS) $(ASFLAGS) -o $@ $^

%.bin: %.elf
	$(OBJCOPY) -O binary $^ $@
