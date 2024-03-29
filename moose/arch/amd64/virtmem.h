#pragma once

#include <moose/types.h>

#define ENTRIES_PER_TABLE 512

#define PAGE_TABLE_INDEX(_x) (((_x) >> 12) & 0x1ff)
#define PAGE_DIRECTORY_INDEX(_x) (((_x) >> 21) & 0x1ff)
#define PAGE_DIRECTORY_PTRT_INDEX(_x) (((_x) >> 30) & 0x1ff)
#define PAGE_PLM4_INDEX(_x) (((_x) >> 39) & 0x1ff)

#define IDENTITY_MAP_SIZE (2 * 1024 * 1024)
#define PML4_BASE_ADDR 0x1000

struct pml4_entry {
    u64 present : 1;
    u64 rw : 1;
    u64 us : 1;
    u64 pwt : 1;
    u64 pcd : 1;
    u64 accessed : 1;
    u64 avl1 : 1;
    u64 reserved : 1;
    u64 avl2 : 4;
    u64 addr : 36;
    u64 reserved2 : 4;
    u64 avl3 : 11;
    u64 xd : 1;
};
static_assert(sizeof(struct pml4_entry) == 8);

struct pdpt_entry {
    u64 present : 1;
    u64 rw : 1;
    u64 us : 1;
    u64 pwt : 1;
    u64 pcd : 1;
    u64 accessed : 1;
    u64 avl1 : 1;
    u64 page_size : 1;
    u64 avl2 : 4;
    u64 addr : 36;
    u64 reserved2 : 4;
    u64 avl3 : 11;
    u64 xd : 1;
};
static_assert(sizeof(struct pdpt_entry) == 8);

struct pd_entry {
    u64 present : 1;
    u64 rw : 1;
    u64 us : 1;
    u64 pwt : 1;
    u64 pcd : 1;
    u64 accessed : 1;
    u64 avl1 : 1;
    u64 page_size : 1;
    u64 avl2 : 4;
    u64 addr : 36;
    u64 reserved2 : 4;
    u64 avl3 : 11;
    u64 xd : 1;
};
static_assert(sizeof(struct pd_entry) == 8);

struct pt_entry {
    u64 present : 1;
    u64 rw : 1;
    u64 us : 1;
    u64 pwt : 1;
    u64 pcd : 1;
    u64 accessed : 1;
    u64 dirty : 1;
    u64 pat : 1;
    u64 global : 1;
    u64 avl1 : 3;
    u64 addr : 36;
    u64 reserved2 : 4;
    u64 avl2 : 7;
    u64 pk : 4;
    u64 xd : 1;
};
static_assert(sizeof(struct pt_entry) == 8);

struct pml4_table {
    struct pml4_entry entries[ENTRIES_PER_TABLE];
};

struct pdptr_table {
    struct pdpt_entry entries[ENTRIES_PER_TABLE];
};

struct page_directory {
    struct pd_entry entries[ENTRIES_PER_TABLE];
};

struct page_table {
    struct pt_entry entries[ENTRIES_PER_TABLE];
};

int alloc_virtual_page(u64 virt_addr);
int alloc_virtual_pages(u64 virt_addr, size_t page_count);
void free_virtual_page(u64 virt_addr);
void free_virtual_pages(u64 virt_addr, size_t page_count);

int map_virtual_page(u64 phys_addr, u64 virt_addr);
int map_virtual_region(u64 phys_base, u64 virt_base, size_t size);
void unmap_virtual_page(u64 virt_addr);
void unmap_virtual_region(u64 virt_addr, size_t size);

void flush_tlb_entry(u64 virt_addr);
