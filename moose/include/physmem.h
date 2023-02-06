#pragma once

#include <types.h>

#define PAGE_SIZE 4096

struct mem_range {
    u64 base;
    u64 size;
};

int init_phys_mem(const struct mem_range *memmap, size_t memmap_size);

ssize_t alloc_page(void);
ssize_t alloc_pages(size_t count);

void free_pages(u64 addr, size_t count);
void free_page(u64 addr);

ssize_t alloc_region(u64 addr, size_t count);
void free_region(u64 addr, size_t count);
