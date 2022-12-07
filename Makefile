BUILD_DIR := build

QEMU := qemu-system-aarch64
QEMU_MACHINE_TYPE := raspi3b
QEMU_RELEASE_ARGS := -d in_asm -display none

CC := aarch64-elf-gcc
LD := aarch64-elf-ld
NM := aarch64-elf-nm
READELF := aarch64-elf-readelf
OBJDUMP := aarch64-elf-objdump
OBJCOPY := aarch64-elf-objcopy

CFLAGS := -Wall -Werror -Wextra -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles

SRCS := kernel/main.s
LINK_SCRIPT := kernel/arch/aarch64/link.x

KERNEL_TARGET := moose.img
KERNEL_ELF    := build/moose.elf

$(shell mkdir -p $(BUILD_DIR))

all: $(KERNEL_TARGET)

$(KERNEL_TARGET): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $^ $@

$(KERNEL_ELF): build/boot.o build/main.o
	$(CC) $(CFLAGS) -T $(LINK_SCRIPT) -o $@ $^

build/boot.o: kernel/arch/aarch64/boot.s
	$(CC) $(CFLAGS) -c $< -o $@

build/main.o: kernel/main.c
	$(CC) $(CFLAGS) -c $< -o $@

qemu: $(KERNEL_TARGET)
	$(QEMU) -M $(QEMU_MACHINE_TYPE) $(QEMU_RELEASE_FLAGS) -kernel $<

clean:
	rm -rf build $(KERNEL_TARGET)

.PHONY: clean qemu

