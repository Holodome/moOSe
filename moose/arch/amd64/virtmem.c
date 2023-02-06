#include <arch/amd64/physmem.h>
#include <arch/amd64/virtmem.h>
#include <kmem.h>
#include <kstdio.h>

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

    memset(VIRT_ADDR((void *)addr), 0, sizeof(struct page_table));

    return (struct page_table *)addr;
}

static struct page_directory *alloc_page_directory(void) {
    ssize_t addr = alloc_page();
    if (addr < 0)
        return NULL;

    memset(VIRT_ADDR((void *)addr), 0, sizeof(struct page_directory));

    return (struct page_directory *)addr;
}

static struct pdptr_table *alloc_pdptr_table(void) {
    ssize_t addr = alloc_page();
    if (addr < 0)
        return NULL;

    memset(VIRT_ADDR((void *)addr), 0, sizeof(struct pdptr_table));

    return (struct pdptr_table *)addr;
}

__attribute__((unused)) static struct pml4_table *alloc_pml4_table(void) {
    ssize_t addr = alloc_page();
    if (addr < 0)
        return NULL;

    memset(VIRT_ADDR((void *)addr), 0, sizeof(struct pdptr_table));

    return (struct pml4_table *)addr;
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

static inline struct pml4_entry *pml4_lookup(struct pml4_table *table,
                                             u64 virt_addr) {
    if (table)
        return &table->entries[PAGE_PLM4_INDEX(virt_addr)];
    return NULL;
}

static struct pml4_table *root_table;

int map_virtual_page(u64 phys_addr, u64 virt_addr) {
    struct pml4_table *pml4_table = VIRT_ADDR(get_pml4_table());
    struct pml4_entry *pml4_entry = pml4_lookup(pml4_table, virt_addr);

    if (!pml4_entry->present) {
        struct pdptr_table *pdptr_table = alloc_pdptr_table();
        if (pdptr_table == NULL)
            return 1;

        pml4_entry->present = 1;
        pml4_entry->rw = 1;
        pml4_entry->addr = (u64)pdptr_table >> 12;
    }

    struct pdptr_table *pdptr_table = VIRT_ADDR((u64)pml4_entry->addr << 12);
    struct pdpt_entry *pdpt_entry = pdpt_lookup(pdptr_table, virt_addr);

    if (!pdpt_entry->present) {
        struct page_directory *page_directory = alloc_page_directory();
        if (page_directory == NULL)
            return 1;

        pdpt_entry->present = 1;
        pdpt_entry->rw = 1;
        pdpt_entry->addr = (u64)page_directory >> 12;
    }

    struct page_directory *page_directory = VIRT_ADDR((u64)pdpt_entry->addr << 12);
    struct pd_entry *pd_entry = pd_lookup(page_directory, virt_addr);

    if (!pd_entry->present) {
        struct page_table *page_table = alloc_page_table();
        if (page_table == NULL)
            return 1;

        pd_entry->present = 1;
        pd_entry->rw = 1;
        pd_entry->addr = (u64)page_table >> 12;
    }

    struct page_table *page_table = VIRT_ADDR((u64)pd_entry->addr << 12);
    struct pt_entry *pt_entry = pt_lookup(page_table, virt_addr);

    pt_entry->present = 1;
    pt_entry->addr = phys_addr >> 12;

    return 0;
}

void unmap_virtual_page(u64 virt_addr) {
    struct pt_entry *entry = get_page_entry(virt_addr);
    if (entry) {
        entry->addr = 0;
        entry->present = 0;
//        flush_tlb_entry(virt_addr);
    }
}

void flush_tlb_entry(u64 virt_addr) {
    __asm__("cli; invlpg (%0); sti" : : "r"(virt_addr));
}

struct pt_entry *get_page_entry(u64 virt_addr) {
    struct pml4_table *pml4_table = VIRT_ADDR(get_pml4_table());
    struct pml4_entry *pml4_entry = pml4_lookup(pml4_table, virt_addr);
    if (pml4_entry == NULL)
        return NULL;

    struct pdptr_table *pdptr_table = VIRT_ADDR((u64)pml4_entry->addr << 12);
    struct pdpt_entry *pdpt_entry = pdpt_lookup(pdptr_table, virt_addr);
    if (pdpt_entry == NULL)
        return NULL;

    struct page_directory *page_directory =
        VIRT_ADDR((u64)pdpt_entry->addr << 12);
    struct pd_entry *pd_entry = pd_lookup(page_directory, virt_addr);
    if (pd_entry == NULL)
        return NULL;

    struct page_table *page_table = VIRT_ADDR((u64)pd_entry->addr << 12);
    struct pt_entry *entry = pt_lookup(page_table, virt_addr);

    return entry;
}

void set_pml4_table(struct pml4_table *table) {
    root_table = VIRT_ADDR(table);
    asm volatile ("movq %%rax, %%cr3" : : "a"(table));
}

struct pml4_table *get_pml4_table(void) { return root_table; }

int init_virt_mem(const struct memmap_entry *memmap, size_t memmap_size) {
    root_table = (struct pml4_table *)DEFAULT_PAGE_TABLES_BASE;

    // preallocate kernel physical space
    if (alloc_region(KERNEL_BASE_ADDR, KERNEL_SIZE / PAGE_SIZE) < 0)
        return 1;

    // preallocate currently used page tables
    if (alloc_region(0, 8) < 0)
        return 1;

    // all physical memory map to 0xffff880000000000
    for (size_t i = 0; i < memmap_size; i++) {
        for (u64 addr = memmap[i].base;
             addr < memmap[i].base + memmap[i].length; addr += PAGE_SIZE) {
            if (map_virtual_page(addr, DIRECT_MEMMAP_BASE + addr))
                return 1;
        }
    }

    // map kernel region
    for (u64 addr = 0; addr < KERNEL_SIZE; addr += PAGE_SIZE) {
        if (map_virtual_page(addr + KERNEL_BASE_ADDR,
                             addr + KERNEL_TEXT_MAP_BASE))
            return 1;
    }

//    // physical memory identity unmap
//    for (u64 addr = 0; addr < IDENTITY_MAP_SIZE; addr += PAGE_SIZE)
//        unmap_virtual_page(addr);

    return 0;
}
