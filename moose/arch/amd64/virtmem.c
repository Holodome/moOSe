#include <arch/amd64/virtmem.h>
#include <arch/amd64/physmem.h>

ssize_t alloc_virtual_page(struct pt_entry *entry) {
    ssize_t addr = alloc_page();
    if (addr < 0)
        return -1;

    entry->addr = addr;
    entry->present = 1;

    return 0;
}

void free_virtual_page(struct pt_entry *entry) {
    if (entry->present)
        free_page(entry->addr);

    entry->present = 0;
}

static inline struct pt_entry *pt_lookup(struct page_table *table, u64 addr) {
    if (table)
        return &table->entries[PAGE_TABLE_INDEX(addr)];
    return NULL;
}

static inline struct pd_entry *pd_lookup(struct page_directory *directory,
                                         u64 addr) {
    if (directory)
        return &directory->entries[PAGE_DIRECTORY_INDEX(addr)];
    return NULL;
}

static inline struct pdpt_entry *pdpt_lookup(struct pdptr_table *table,
                                           u64 addr) {
    if (table)
        return &table->entries[PAGE_DIRECTORY_PTRT_INDEX(addr)];
    return NULL;
}
