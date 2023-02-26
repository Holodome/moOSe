/*
 * Macros used both in C and Assembly code
 */
#pragma once

#define PAGE_SIZE 4096
#define PAGE_SIZE_BITS 12

#define PHYSMEM_VIRTUAL_BASE 0xffff880000000000
#define KERNEL_PHYSICAL_BASE 0x100000
#define KERNEL_VIRTUAL_BASE (PHYSMEM_VIRTUAL_BASE + KERNEL_PHYSICAL_BASE)
#define KERNEL_SIZE (2 * 1024 * 1024)
#define KBOOT_SIZE PAGE_SIZE

#define FIXUP_ADDR(_mem) ((_mem) + PHYSMEM_VIRTUAL_BASE)
#define FIXUP_PTR(_ptr) (((void *)(_ptr) + PHYSMEM_VIRTUAL_BASE))
