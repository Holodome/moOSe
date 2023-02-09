# moOSe

An unix-like operating system with aim at simplicity.

## Features 

* amd64 kernel
* Custom bootloader
* MBR HDD image
* FAT12, FAT16, FAT32 support
* ATA PIO
* 8042 keyboard controller
* VGA display output
* Physical memory allocator
* Virtual memory allocator
* brk and malloc
* System time using RTC
* Interactive shell

## Compiling & running

*x86_64-elf-gcc* crosscompilation toolchain and *python3* are required for local build.
*qemu-system-x86_64* emulator is required for running.
Currently running only in qemu is supported.

To produce OS image either run `make` or use `docker buildx build 
-f Dockerfile  --output . .`

OS image *moose.img* is a virtual hard disk formatted with MBR.

to run pass it to qemu:
```
qemu-system-x86_64 -hda moose.img
```

Alternatively, `make qemu` can be used to build and launch qemu.
