# moOSe

An unix-like operating system with aim at simplicity.

## Features 

* amd64 kernel
* Custom bootloader
* FAT12, FAT16, FAT32, ext2, ramfs filesystems
* ATA IO
* Multilevel feedback queue scheduler
* System calls
* Basic IP networking

## Compiling & running

*x86_64-elf-gcc* crosscompilation toolchain and *python3* are required for local build.
*qemu-system-x86_64* emulator is required for running.
Currently running only in qemu is supported.

To produce OS image either run `make` or use `docker buildx build 
-f Dockerfile  --output . .`

OS image *moose.img* is a virtual hard disk formatted with MBR.

to run pass it to qemu:
```
qemu-system-x86_64 -d guest_errors \
	-device pci-bridge,id=bridge1,bus=pci.0,chassis_nr=4 \
	-device rtl8139,netdev=moose0,bus=pci.0 -netdev user,id=moose0 \
	-drive file=moose.img,format=raw,index=0,if=ide \
	-no-reboot
```

Alternatively, `make qemu` can be used to build and launch qemu.
