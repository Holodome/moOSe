#include <arch/amd64/asm.h>
#include <arch/amd64/virtmem.h>
#include <assert.h>
#include <kernel.h>
#include <kmem.h>
#include <arch/cpu.h>
#include <kstdio.h>
#include <physmem.h>

int alloc_virtual_page(u64 virt_addr) {
    ssize_t addr = alloc_page();
    if (addr < 0)
        return 1;

    map_virtual_page(addr, virt_addr);

    return 0;
}

void free_virtual_page(u64 virt_addr) {
    struct pt_entry *pt_entry = get_page_entry(virt_addr);
    if (pt_entry != NULL && pt_entry->present) {
        free_page(pt_entry->addr);
        unmap_virtual_page(virt_addr);
    }
}

static void *alloc_page_table(void) {
    ssize_t addr = alloc_page();
    if (addr < 0)
        return NULL;

    memset(FIXUP_PTR((void *)addr), 0, PAGE_SIZE);

    return (void *)addr;
}

static inline struct pt_entry *pt_lookup(struct page_table *table,
                                         u64 virt_addr) {
    return &table->entries[PAGE_TABLE_INDEX(virt_addr)];
}

static inline struct pd_entry *pd_lookup(struct page_directory *directory,
                                         u64 virt_addr) {
    return &directory->entries[PAGE_DIRECTORY_INDEX(virt_addr)];
}

static inline struct pdpt_entry *pdpt_lookup(struct pdptr_table *table,
                                             u64 virt_addr) {
    return &table->entries[PAGE_DIRECTORY_PTRT_INDEX(virt_addr)];
}

static inline struct pml4_entry *pml4_lookup(struct pml4_table *table,
                                             u64 virt_addr) {
    return &table->entries[PAGE_PLM4_INDEX(virt_addr)];
}

static struct pml4_table *root_table;

int map_virtual_page(u64 phys_addr, u64 virt_addr) {
    struct pml4_table *pml4_table = FIXUP_PTR(get_pml4_table());
    struct pml4_entry *pml4_entry = pml4_lookup(pml4_table, virt_addr);

    if (!pml4_entry->present) {
        struct pdptr_table *pdptr_table = alloc_page_table();
        if (pdptr_table == NULL)
            return -1;

        pml4_entry->present = 1;
        pml4_entry->rw = 1;
        pml4_entry->addr = (u64)pdptr_table >> PAGE_SIZE_BITS;
    }

    struct pdptr_table *pdptr_table =
        FIXUP_PTR((u64)pml4_entry->addr << PAGE_SIZE_BITS);
    struct pdpt_entry *pdpt_entry = pdpt_lookup(pdptr_table, virt_addr);

    if (!pdpt_entry->present) {
        struct page_directory *page_directory = alloc_page_table();
        if (page_directory == NULL)
            return -1;

        pdpt_entry->present = 1;
        pdpt_entry->rw = 1;
        pdpt_entry->addr = (u64)page_directory >> PAGE_SIZE_BITS;
    }

    struct page_directory *page_directory =
        FIXUP_PTR((u64)pdpt_entry->addr << PAGE_SIZE_BITS);
    struct pd_entry *pd_entry = pd_lookup(page_directory, virt_addr);

    if (!pd_entry->present) {
        struct page_table *page_table = alloc_page_table();
        if (page_table == NULL)
            return -1;

        pd_entry->present = 1;
        pd_entry->rw = 1;
        pd_entry->addr = (u64)page_table >> PAGE_SIZE_BITS;
    }

    struct page_table *page_table =
        FIXUP_PTR((u64)pd_entry->addr << PAGE_SIZE_BITS);
    struct pt_entry *pt_entry = pt_lookup(page_table, virt_addr);

    pt_entry->present = 1;
    pt_entry->addr = phys_addr >> PAGE_SIZE_BITS;

    return 0;
}

int map_virtual_region(u64 phys_base, u64 virt_base, size_t count) {
    for (u64 addr = 0; addr < count * PAGE_SIZE; addr += PAGE_SIZE)
        if (map_virtual_page(phys_base + addr, virt_base + addr))
            return -1;

    return 0;
}

void unmap_virtual_page(u64 virt_addr) {
    struct pt_entry *entry = get_page_entry(virt_addr);
    if (entry) {
        entry->addr = 0;
        entry->present = 0;
        flush_tlb_entry(virt_addr);
    }
}

void unmap_virtual_region(u64 virt_addr, size_t count) {
    for (u64 addr = 0; addr < count * PAGE_SIZE; addr += PAGE_SIZE)
        unmap_virtual_page(virt_addr + addr);
}

void flush_tlb_entry(u64 virt_addr) {
    unsigned long flags;
    irq_save(&flags);
    asm volatile("invlpg (%0)" : : "r"(virt_addr) : "memory");
    irq_restore(flags);
}

struct pt_entry *get_page_entry(u64 virt_addr) {
    struct pml4_table *pml4_table = FIXUP_PTR(get_pml4_table());
    struct pml4_entry *pml4_entry = pml4_lookup(pml4_table, virt_addr);
    if (!pml4_entry->present)
        return NULL;

    struct pdptr_table *pdptr_table =
        FIXUP_PTR((u64)pml4_entry->addr << PAGE_SIZE_BITS);
    struct pdpt_entry *pdpt_entry = pdpt_lookup(pdptr_table, virt_addr);
    if (!pdpt_entry->present)
        return NULL;

    struct page_directory *page_directory =
        FIXUP_PTR((u64)pdpt_entry->addr << PAGE_SIZE_BITS);
    struct pd_entry *pd_entry = pd_lookup(page_directory, virt_addr);
    if (!pd_entry->present)
        return NULL;

    struct page_table *page_table =
        FIXUP_PTR((u64)pd_entry->addr << PAGE_SIZE_BITS);
    struct pt_entry *entry = pt_lookup(page_table, virt_addr);

    return entry;
}

void set_pml4_table(struct pml4_table *table) {
    root_table = FIXUP_PTR(table);
    write_cr3((u64)table);
}

struct pml4_table *get_pml4_table(void) { return root_table; }

int init_virt_mem(const struct mem_range *ranges, size_t ranges_size) {
    root_table = (struct pml4_table *)PML4_BASE_ADDR;

    // preallocate kernel physical space
    if (alloc_region(KERNEL_PHYSICAL_BASE, KERNEL_SIZE >> PAGE_SIZE_BITS))
        return -1;

    // preallocate currently used page tables
    if (alloc_region(0, 8))
        return -1;

    // all physical memory map to PHYSMEM_VIRTUAL_BASE
    for (size_t i = 0; i < ranges_size; i++) {
        if (map_virtual_region(ranges[i].base,
                               PHYSMEM_VIRTUAL_BASE + ranges[i].base,
                               ranges[i].size >> PAGE_SIZE_BITS))
            return -1;
    }

    // physical memory identity unmap
    unmap_virtual_region(0x0, IDENTITY_MAP_SIZE >> PAGE_SIZE_BITS);

    return 0;
}
