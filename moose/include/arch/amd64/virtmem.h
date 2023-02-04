#pragma once

#include <types.h>

#define ENTRIES_PER_TABLE 512

#define PAGE_OFFSET(_x) ((_x) & 0xfff)
#define PAGE_TABLE_INDEX(_x) (((_x) >> 12) & 0x1ff)
#define PAGE_DIRECTORY_INDEX(_x) (((_x) >> 21) & 0x1ff)
#define PAGE_DIRECTORY_PTRT_INDEX(_x) (((_x) >> 30) & 0x1ff)
#define PAGE_PLM4_INDEX(_x) (((_x) >> 39) & 0x1ff)

/*
 * Virtual paging entry bits
 * present - present bit
 * rw - read/write
 * us - user/supervisor
 * pwt - write through
 * pcd - cache disable
 * accessed - page was accessed
 * dirty - dirty bit
 * page_size
 * global
 * pat - page attribute table
 * avl - available
 * pk - protection key
 * xd - execute disable
 */

struct plm4_entry {
    u64 present : 1;
    u64 rw : 1;
    u64 us : 1;
    u64 pwt : 1;
    u64 pcd : 1;
    u64 accessed : 1;
    u64 dirty : 1;
    u64 page_size : 1;
    u64 global : 1;
    u64 avl1 : 3;
    u64 pat : 1;
    u64 reserved1 : 17;
    // page directory address
    u64 addr : 9;
    u64 reserved2 : 13;
    u64 avl2 : 7;
    u64 pk : 4;
    u64 xd : 1;
};
static_assert(sizeof(struct plm4_entry) == 8);

struct pdpt_entry {
    u64 present : 1;
    u64 rw : 1;
    u64 us : 1;
    u64 pwt : 1;
    u64 pcd : 1;
    u64 accessed : 1;
    u64 dirty : 1;
    u64 page_size : 1;
    u64 global : 1;
    u64 avl1 : 3;
    u64 pat : 1;
    u64 reserved1 : 17;
    // page directory address
    u64 addr : 9;
    u64 reserved2 : 13;
    u64 avl2 : 7;
    u64 pk : 4;
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
    u64 dirty : 1;
    u64 page_size : 1;
    u64 global : 1;
    u64 avl1 : 3;
    u64 pat : 1;
    u64 reserved1 : 8;
    // page table address
    u64 addr : 9;
    u64 reserved2 : 22;
    u64 avl2 : 7;
    u64 pk : 4;
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
    u64 page_size : 1;
    u64 global : 1;
    u64 avl1 : 4;
    // page offset
    u64 addr : 12;
    u64 reserved2 : 27;
    u64 avl2 : 7;
    u64 pk : 4;
    u64 xd : 1;
};
static_assert(sizeof(struct pt_entry) == 8);

struct plm4_table {
    struct plm4_entry entries[ENTRIES_PER_TABLE];
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

ssize_t alloc_virtual_page(struct pt_entry *entry);
void free_virtual_page(struct pt_entry *entry);

void map_virtual_page(u64 phys_addr, u64 virt_addr);
void set_plm4_table(struct plm4_table *table);
struct plm4_table *get_plm4_table(void);
