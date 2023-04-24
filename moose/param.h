/*
 * Macros used both in C and Assembly code
 */
#pragma once

#define PAGE_SIZE 4096
#define PAGE_SIZE_BITS 12

#define PHYSMEM_VIRTUAL_BASE 0xffff880000000000
#define KERNEL_PHYSICAL_BASE 0x100000
#define KERNEL_VIRTUAL_BASE (PHYSMEM_VIRTUAL_BASE + KERNEL_PHYSICAL_BASE)
#define KBOOT_SIZE PAGE_SIZE

#define FIXUP_ADDR(_mem) ((_mem) + PHYSMEM_VIRTUAL_BASE)
#define FIXUP_PTR(_ptr) (((void *)(_ptr) + PHYSMEM_VIRTUAL_BASE))

#define ADDR_TO_PHYS(_mem) ((_mem)-PHYSMEM_VIRTUAL_BASE)
#define PTR_TO_PHYS(_ptr) (((void *)(_ptr)-PHYSMEM_VIRTUAL_BASE))

#define MMIO_VIRTUAL_BASE 0xffffe90000000000

#define KERNEL_CS 0x08
#define KERNEL_DS 0x10
#define USER_DS   0x18
#define USER_CS   0x20
#define TSS_SEL   0x28
#define TSS_SEL1  0x30


#define KERNEL_INITIAL_STACK 0x90000
