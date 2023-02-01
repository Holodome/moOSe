#pragma once

#include <types.h>

#define PAGE_SIZE 4096
#define KERNEL_BASE_ADDR 0x100000
#define KERNEL_SIZE (2 * 1024 * 1024)
#define MEMORY_BASE_ADDR (KERNEL_BASE_ADDR + KERNEL_SIZE)

int init_phys_manager(void);