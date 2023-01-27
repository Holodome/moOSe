# Select the toolchain toD compile with
CROSSCOMPILE = x86_64-elf-

CC      := $(CROSSCOMPILE)gcc
LD      := $(CROSSCOMPILE)ld
AS      := $(CROSSCOMPILE)as
OBJCOPY := $(CROSSCOMPILE)objcopy
QEMU    := qemu-system-x86_64
GDB     := x86_64-elf-gdb

export CC LD AS OBJCOPY

ASFLAGS = -msyntax=att --warn --fatal-warnings
CFLAGS  = -Wall -Werror -Wextra -std=gnu11 -ffreestanding -nostdlib -nostartfiles -Wl,-r \
			-Imoose/include -O2

ifneq ($(DEBUG),)
	ASFLAGS += -g
	CFLAGS += -ggdb -O0
endif

export ASFLAGS CFLAGS

TARGET_IMG := moose.img

all: $(TARGET_IMG)

$(TARGET_IMG): moose/moose.img
	cp $< $@

qemu: all
	$(QEMU) -d guest_errors -hda $(TARGET_IMG)

qemu-debug: all
	$(QEMU) -s -S -fda moose.img -d guest_errors & 
  	$(GDB) -ex "target remote localhost:1234" -ex "symbol-file moose/arch/boot/adm64/stage2.elf"

clean:
	rm $(shell find . -name "*.o" \
		-o -name "*.d" \
		-o -name "*.bin" \
		-o -name "*.elf" \
		-o -name "*.i" \
		-o -name "*.img")

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.S
	$(CC) $(CFLAGS) -o $@ $^

%.bin: %.elf
	$(OBJCOPY) -O binary $^ $@

%.i: %.c
	$(CC) $(CFLAGS) -E -o $@ $^

include moose/Makefile
