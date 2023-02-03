#pragma once

#include <types.h>

#define PAGE_SIZE 4096
#define KERNEL_BASE_ADDR 0x100000
#define KERNEL_SIZE (2 * 1024 * 1024)
#define MEMORY_BASE_ADDR (KERNEL_BASE_ADDR + KERNEL_SIZE)

// uses <arch/amd64/memory_map.h>
int init_phys_mem(void);

ssize_t alloc_page(void);
ssize_t alloc_pages(size_t count);

void free_pages(u64 addr, size_t count);
void free_page(u64 addr);
