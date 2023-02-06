#pragma once

#include <arch/amd64/memory_map.h>
#include <types.h>

#define PAGE_SIZE 4096

int init_phys_mem(const struct memmap_entry *memmap, size_t memmap_size);

ssize_t alloc_page(void);
ssize_t alloc_pages(size_t count);

void free_pages(u64 addr, size_t count);
void free_page(u64 addr);

ssize_t alloc_region(u64 addr, size_t count);
void free_region(u64 addr, size_t count);
