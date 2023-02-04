#include <arch/amd64/virtmem.h>
#include <arch/amd64/physmem.h>
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

__attribute__((unused)) static struct plm4_table *alloc_plm4_table(void) {
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
static u64 phys_map_base;

int map_virtual_page(u64 phys_addr, u64 virt_addr) {
    struct plm4_table *plm4_table = get_plm4_table();
    struct plm4_entry *plm4_entry = plm4_lookup(plm4_table, virt_addr);

    if (!plm4_entry->present) {
        struct pdptr_table *pdptr_table = alloc_pdptr_table();
        if (pdptr_table == NULL)
            return 1;

        plm4_entry->present = 1;
        plm4_entry->rw = 1;
        plm4_entry->addr = (u64) pdptr_table >> 12;
    }

    struct pdptr_table *pdptr_table =
        (struct pdptr_table *) ((u64) plm4_entry->addr << 12);
    struct pdpt_entry *pdpt_entry = pdpt_lookup(pdptr_table, virt_addr);

    if (!pdpt_entry->present) {
        struct page_directory *page_directory = alloc_page_directory();
        if (page_directory == NULL)
            return 1;

        pdpt_entry->present = 1;
        pdpt_entry->rw = 1;
        pdpt_entry->addr = (u64) page_directory >> 12;
    }

    struct page_directory *page_directory =
        (struct page_directory *) ((u64) pdpt_entry->addr << 12);
    struct pd_entry *pd_entry = pd_lookup(page_directory, virt_addr);

    if (!pd_entry->present) {
        struct page_table *page_table = alloc_page_table();
        if (page_table == NULL)
            return 1;

        pd_entry->present = 1;
        pd_entry->rw = 1;
        pd_entry->addr = (u64) page_table >> 12;
    }

    struct page_table *page_table =
        (struct page_table *) ((u64)pd_entry->addr << 12);
    struct pt_entry *pt_entry = pt_lookup(page_table, virt_addr);

    pt_entry->present = 1;
    pt_entry->addr = phys_addr >> 12;

    return 0;
}

void unmap_virtual_page(u64 virt_addr) {
    struct pt_entry *entry = get_page_entry(virt_addr);
    if (entry)
        entry->present = 0;
}

struct pt_entry *get_page_entry(u64 virt_addr) {
    struct plm4_table *plm4_table = get_plm4_table();
    struct plm4_entry *plm4_entry = plm4_lookup(plm4_table, virt_addr);
    if (plm4_entry == NULL)
        return NULL;

    struct pdptr_table *pdptr_table =
        (struct pdptr_table *) ((u64) plm4_entry->addr << 12);
    struct pdpt_entry *pdpt_entry = pdpt_lookup(pdptr_table, virt_addr);
    if (pdpt_entry == NULL)
        return NULL;

    struct page_directory *page_directory =
        (struct page_directory *) ((u64) pdpt_entry->addr << 12);
    struct pd_entry *pd_entry = pd_lookup(page_directory, virt_addr);
    if (pd_entry == NULL)
        return NULL;

    struct page_table *page_table =
        (struct page_table *) ((u64) pd_entry->addr << 12);
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

int init_virt_mem(const struct memmap_entry *memmap, size_t memmap_size) {
    root_table = (struct plm4_table *) DEFAULT_PAGE_TABLES_BASE;
    phys_map_base = 0x0;

    // preallocate kernel physical space
    if (alloc_region(KERNEL_BASE_ADDR, KERNEL_SIZE / PAGE_SIZE) < 0)
        return 1;

    // preallocate currently used page tables
    if (alloc_region(0, 32) < 0)
        return 1;

    // phys memory identity map
    for (size_t i = 0; i < memmap_size; i++) {
        if (memmap[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
            for (u64 addr = memmap[i].base;
                 addr < memmap[i].base + memmap[i].length;
                 addr += PAGE_SIZE) {
                if (map_virtual_page(addr, addr))
                    return 1;
            }
        }
    }

    // phys memory map to 0xffff880000000000
    for (size_t i = 0; i < memmap_size; i++) {
        if (memmap[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
            for (u64 addr = memmap[i].base;
                 addr < memmap[i].base + memmap[i].length;
                 addr += PAGE_SIZE) {
                if (map_virtual_page(addr, DIRECT_MEMORY_MAPPING_BASE + addr))
                    return 1;
            }
        }
    }

    for (u64 virt = KERNEL_TEXT_MAPPING_BASE, addr = KERNEL_BASE_ADDR;
         addr < KERNEL_SIZE;
         addr += PAGE_SIZE, virt += PAGE_SIZE)
        if (map_virtual_page(addr, virt))
            return 1;

    phys_map_base = DIRECT_MEMORY_MAPPING_BASE;

    return 0;
}
