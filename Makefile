ifndef VERBOSE 
	Q = @
else
	Q = 
endif 

# Select the toolchain to compile with
CROSSCOMPILE = x86_64-elf-

CC      := $(CROSSCOMPILE)gcc
LD      := $(CROSSCOMPILE)ld
AS      := $(CROSSCOMPILE)as
OBJCOPY := $(CROSSCOMPILE)objcopy
QEMU    := qemu-system-x86_64
GDB     := x86_64-elf-gdb

DEPFLAGS = -MT $@ -MMD -MP -MF $(subst .o,.d,$@)
ASFLAGS = -msyntax=att --warn --fatal-warnings
LDFLAGS = -Map $(subst .elf,.map,$@)

CFLAGS  = -I.\
		  -Wall -Werror -Wextra -Wno-sign-compare -Wpacked \
		  -Os -g -std=gnu11 -fno-strict-aliasing -fno-strict-overflow \
		  -ffreestanding -nostdlib -nostartfiles \
		  -Wl,-r -mno-sse -mno-sse2 -mno-sse3 -mcmodel=large -mno-red-zone 

TARGET_IMG := moose.img

all: $(TARGET_IMG)

re: 
	$(MAKE) clean
	$(MAKE) all

$(TARGET_IMG): moose/moose.img
	@echo Wrote target to $@
	$(Q)cp $< $@

qemu: all
	$(QEMU) -d guest_errors \
	-device pci-bridge,id=bridge1,bus=pci.0,chassis_nr=4 \
	-device rtl8139,netdev=moose0,bus=pci.0 -netdev user,id=moose0 \
	-drive file=$(TARGET_IMG),format=raw,index=0,if=ide

format:
	$(Q)find moose \( -name "*.c" -o -name "*.h" \) -exec clang-format -i {} \;

clean:
	$(Q)rm -f $(shell find . -name "*.o" \
		-o -name "*.d" \
		-o -name "*.bin" \
		-o -name "*.elf" \
		-o -name "*.i" \
		-o -name "*.map" \
		-o -name "*.out" \
		-o -name "*.img")


include moose/Makefile
-include $(shell find . -name "*.d")

%.o: %.c
	@echo "CC $<"
	$(Q)$(CC) $(CFLAGS) $(DEPFLAGS) -o $@ $<

%.o: %.S
	@echo "AS $<"
	$(Q)$(CC) $(CFLAGS) -Wa,--64 -o $@ $<

%.bin: %.elf
	@echo "OBJCOPY $@"
	$(Q)$(OBJCOPY) -O binary $^ $@

%.ld.out: %.ld
	@echo "CPP $<"
	$(Q)gcc -CC -E -P -x c -I. $< > $@

%.i: %.c
	$(Q)$(CC) $(CFLAGS) -E -o $@ $^

.PHONY: qemu all clean format
