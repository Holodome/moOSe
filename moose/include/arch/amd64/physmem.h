#pragma once

#include <types.h>

#define PAGE_SIZE 4096
#define KERNEL_BASE_ADDR 0x100000
#define KERNEL_SIZE (2 * 1024 * 1024)
#define MEMORY_BASE_ADDR (KERNEL_BASE_ADDR + KERNEL_SIZE)

struct page_block {
    u64 addr;
    size_t count;
};

int init_phys_manager(void);

struct page_block *alloc_page_block(size_t count);
u64 alloc_page(void);
u64 alloc_pages(size_t count);

void free_page_block(struct page_block *page);
void free_page(void *addr);
