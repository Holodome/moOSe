#pragma once

#include <types.h>
#include <arch/amd64/memory_map.h>

#define PAGE_SIZE 4096
#define KERNEL_BASE_ADDR 0x1000000
#define KERNEL_SIZE (4 * 1024 * 1024)
#define DEFAULT_PAGE_TABLES_BASE 0x1000

int init_phys_mem(const struct memmap_entry *memmap, size_t memmap_size);

ssize_t alloc_page(void);
ssize_t alloc_pages(size_t count);

void free_pages(u64 addr, size_t count);
void free_page(u64 addr);

ssize_t alloc_region(u64 addr, size_t count);
void free_region(u64 addr, size_t count);
