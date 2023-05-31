#pragma once

#include <moose/types.h>

struct mem_range {
    u64 base;
    u64 size;
};

// 1 - 2^BUDDY_MAX_ORDER pages in block
#define MAX_ORDER 12
#define MAX_BLOCK_SIZE_BITS (PAGE_SIZE_BITS + MAX_ORDER)

// largest page block for buddy allocator
#define MAX_BLOCK_SIZE ((PAGE_SIZE << (MAX_ORDER)))

int init_phys_mem(const struct mem_range *ranges, size_t ranges_size);

ssize_t alloc_page(void);
ssize_t alloc_pages(u32 order);

void free_pages(u64 addr, u32 order);
void free_page(u64 addr);

int alloc_region(u64 addr, u64 count);
void free_region(u64 addr, u64 count);
