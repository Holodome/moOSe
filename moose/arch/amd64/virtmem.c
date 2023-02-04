#include <arch/amd64/virtmem.h>
#include <arch/amd64/physmem.h>
#include <kmem.h>

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

static struct page_table *alloc_page_table(void) {
    ssize_t addr = alloc_page();
    if (addr < 0)
        return NULL;

    memset((void *) addr, 0, sizeof(struct page_table));

    return (struct page_table *) addr;
}

static struct page_directory *alloc_page_directory(void) {
    ssize_t addr = alloc_page();
    if (addr < 0)
        return NULL;

    memset((void *) addr, 0, sizeof(struct page_directory));

    return (struct page_directory *) addr;
}

static struct pdptr_table *alloc_pdptr_table(void) {
    ssize_t addr = alloc_page();
    if (addr < 0)
        return NULL;

    memset((void *) addr, 0, sizeof(struct pdptr_table));

    return (struct pdptr_table *) addr;
}

static struct plm4_table *alloc_plm4_table(void) {
    ssize_t addr = alloc_page();
    if (addr < 0)
        return NULL;

    memset((void *) addr, 0, sizeof(struct pdptr_table));

    return (struct plm4_table *) addr;
}

static inline struct pt_entry *pt_lookup(struct page_table *table,
                                         u64 virt_addr) {
    if (table)
        return &table->entries[PAGE_TABLE_INDEX(virt_addr)];
    return NULL;
}

static inline struct pd_entry *pd_lookup(struct page_directory *directory,
                                         u64 virt_addr) {
    if (directory)
        return &directory->entries[PAGE_DIRECTORY_INDEX(virt_addr)];
    return NULL;
}

static inline struct pdpt_entry *pdpt_lookup(struct pdptr_table *table,
                                           u64 virt_addr) {
    if (table)
        return &table->entries[PAGE_DIRECTORY_PTRT_INDEX(virt_addr)];
    return NULL;
}

static inline struct plm4_entry *plm4_lookup(struct plm4_table *table,
                                             u64 virt_addr) {
    if (table)
        return &table->entries[PAGE_PLM4_INDEX(virt_addr)];
    return NULL;
}

static struct plm4_table *root_table;

void map_virtual_page(u64 phys_addr, u64 virt_addr) {
    struct plm4_table *plm4_table = get_plm4_table();

    struct plm4_entry *plm4_entry = plm4_lookup(plm4_table, virt_addr);
    if (!plm4_entry->present) {
        struct pdptr_table *pdptr_table = alloc_pdptr_table();
        if (pdptr_table == NULL)
            return;

        plm4_entry->present = 1;
        plm4_entry->rw = 1;
        plm4_entry->addr = (u64) pdptr_table;
    }

    struct pdptr_table *pdptr_table = (struct pdptr_table *) (u64)plm4_entry->addr;
    struct pdpt_entry *pdpt_entry = pdpt_lookup(pdptr_table, virt_addr);
    if (!pdpt_entry->present) {
        struct page_directory *page_directory = alloc_page_directory();
        if (page_directory == NULL)
            return;

        pdpt_entry->present = 1;
        pdpt_entry->rw = 1;
        pdpt_entry->addr = (u64) page_directory;
    }

    struct page_directory *page_directory =
        (struct page_directory *) (u64)pdpt_entry->addr;
    struct pd_entry *pd_entry = pd_lookup(page_directory, virt_addr);
    if (!pd_entry->present) {
        struct page_table *page_table = alloc_page_table();
        if (page_table == NULL)
            return;

        pd_entry->present = 1;
        pd_entry->rw = 1;
        pd_entry->addr = (u64) page_directory;
    }

    struct page_table *page_table = (struct page_table *) (u64)pd_entry->addr;
    struct pt_entry *pt_entry = pt_lookup(page_table, virt_addr);

    pt_entry->present = 1;
    pt_entry->addr = phys_addr;
}

struct pt_entry *get_page_entry(u64 virt_addr) {
    struct plm4_table *plm4_table = get_plm4_table();

    struct plm4_entry *plm4_entry = plm4_lookup(plm4_table, virt_addr);
    struct page_directory *page_directory =
        (struct page_directory *) (u64)plm4_entry->addr;

    struct pd_entry *pd_entry = pd_lookup(page_directory, virt_addr);
    struct page_table *page_table = (struct page_table *) (u64)pd_entry->addr;

    struct pt_entry *entry = pt_lookup(page_table, virt_addr);

    return entry;
}

void set_plm4_table(struct plm4_table *table) {
    if (table == NULL)
        return;

    root_table = table;
    __asm__("movq %%rax, %%cr3" : : "a"(root_table));
}

struct plm4_table *get_plm4_table(void) {
    return root_table;
}
