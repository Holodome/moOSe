#pragma once

#include <types.h>

#define PAGE_SIZE 4096
<<<<<<< HEAD
=======
#define PAGE_SIZE_LOG 12

#define KERNEL_BASE_ADDR 0x100000
#define KERNEL_SIZE (1024 * 1024)
#define DEFAULT_PAGE_TABLES_BASE 0x1000
>>>>>>> 69fbf70 (kernel: physical allocator is working)

struct mem_range {
    u64 base;
    u64 size;
};

int init_phys_mem(const struct mem_range *memmap, size_t memmap_size);

ssize_t alloc_page(void);
ssize_t alloc_pages(size_t order);

void free_pages(u64 addr, size_t count);
void free_page(u64 addr);

int alloc_region(u64 addr, size_t count);
void free_region(u64 addr, size_t count);
