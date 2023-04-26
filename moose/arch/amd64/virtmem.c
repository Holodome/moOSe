#include <moose/arch/amd64/asm.h>
#include <moose/arch/amd64/virtmem.h>
#include <moose/arch/cpu.h>
#include <moose/mm/physmem.h>
#include <moose/param.h>
#include <moose/string.h>

static ssize_t alloc_page_table(void) {
    ssize_t addr = alloc_page();
    if (addr < 0)
        return -1;

    memset((void *)FIXUP_ADDR(addr), 0, PAGE_SIZE);
    return addr;
}

static struct pml4_table *get_pml4_table(void) {
    return FIXUP_PTR(PML4_BASE_ADDR);
}

static struct pt_entry *pt_lookup(struct page_table *table, u64 virt_addr) {
    return table->entries + PAGE_TABLE_INDEX(virt_addr);
}

static struct pd_entry *pd_lookup(struct page_directory *directory,
                                  u64 virt_addr) {
    return &directory->entries[PAGE_DIRECTORY_INDEX(virt_addr)];
}

static struct pdpt_entry *pdpt_lookup(struct pdptr_table *table,
                                      u64 virt_addr) {
    return table->entries + PAGE_DIRECTORY_PTRT_INDEX(virt_addr);
}

static struct pml4_entry *pml4_lookup(struct pml4_table *table, u64 virt_addr) {
    return table->entries + PAGE_PLM4_INDEX(virt_addr);
}

static struct pdptr_table *pdptr_from_pml4e(const struct pml4_entry *e) {
    return FIXUP_PTR((u64)e->addr << PAGE_SIZE_BITS);
}

static struct page_directory *pdir_from_pdpte(const struct pdpt_entry *e) {
    return FIXUP_PTR((u64)e->addr << PAGE_SIZE_BITS);
}

static struct page_table *pt_from_pdire(const struct pd_entry *e) {
    return FIXUP_PTR((u64)e->addr << PAGE_SIZE_BITS);
}

static struct pt_entry *get_page_entry(u64 virt_addr) {
    struct pml4_table *pml4_table = get_pml4_table();
    struct pml4_entry *pml4_entry = pml4_lookup(pml4_table, virt_addr);
    if (!pml4_entry->present)
        return NULL;

    struct pdptr_table *pdptr_table = pdptr_from_pml4e(pml4_entry);
    struct pdpt_entry *pdpt_entry = pdpt_lookup(pdptr_table, virt_addr);
    if (!pdpt_entry->present)
        return NULL;

    struct page_directory *page_directory = pdir_from_pdpte(pdpt_entry);
    struct pd_entry *pd_entry = pd_lookup(page_directory, virt_addr);
    if (!pd_entry->present)
        return NULL;

    struct page_table *page_table = pt_from_pdire(pd_entry);
    struct pt_entry *entry = pt_lookup(page_table, virt_addr);

    return entry;
}

int map_virtual_page(u64 phys_addr, u64 virt_addr) {
    struct pml4_table *pml4_table = get_pml4_table();
    struct pml4_entry *pml4_entry = pml4_lookup(pml4_table, virt_addr);

    if (!pml4_entry->present) {
        ssize_t pdptr_table = alloc_page_table();
        if (pdptr_table < 0)
            return -1;

        pml4_entry->present = 1;
        pml4_entry->rw = 1;
        pml4_entry->addr = (u64)pdptr_table >> PAGE_SIZE_BITS;
        pml4_entry->us = 1;
    }

    struct pdptr_table *pdptr_table = pdptr_from_pml4e(pml4_entry);
    struct pdpt_entry *pdpt_entry = pdpt_lookup(pdptr_table, virt_addr);

    if (!pdpt_entry->present) {
        ssize_t page_directory = alloc_page_table();
        if (page_directory < 0)
            return -1;

        pdpt_entry->present = 1;
        pdpt_entry->rw = 1;
        pdpt_entry->addr = (u64)page_directory >> PAGE_SIZE_BITS;
        pdpt_entry->us = 1;
    }

    struct page_directory *page_directory = pdir_from_pdpte(pdpt_entry);
    struct pd_entry *pd_entry = pd_lookup(page_directory, virt_addr);

    if (!pd_entry->present) {
        ssize_t page_table = alloc_page_table();
        if (page_table < 0)
            return -1;

        pd_entry->present = 1;
        pd_entry->rw = 1;
        pd_entry->addr = (u64)page_table >> PAGE_SIZE_BITS;
        pd_entry->us = 1;
    }

    struct page_table *page_table = pt_from_pdire(pd_entry);
    struct pt_entry *pt_entry = pt_lookup(page_table, virt_addr);

    pt_entry->present = 1;
    pt_entry->addr = phys_addr >> PAGE_SIZE_BITS;
    pt_entry->us = 1;
    pt_entry->rw = 1;

    flush_tlb_entry(virt_addr);
    flush_tlb_entry(phys_addr);

    return 0;
}

int map_virtual_region(u64 phys_base, u64 virt_base, size_t count) {
    for (u64 addr = 0; addr < count * PAGE_SIZE; addr += PAGE_SIZE) {
        if (map_virtual_page(phys_base + addr, virt_base + addr))
            return -1;
    }

    return 0;
}

void unmap_virtual_page(u64 virt_addr) {
    struct pt_entry *entry = get_page_entry(virt_addr);
    if (entry) {
        entry->present = 0;
        /* memset((void *)entry, 0, sizeof(*entry)); */
        flush_tlb_entry(virt_addr);
    }
}

void unmap_virtual_region(u64 virt_addr, size_t count) {
    for (u64 addr = 0; addr < count * PAGE_SIZE; addr += PAGE_SIZE)
        unmap_virtual_page(virt_addr + addr);
}

void flush_tlb_entry(u64 virt_addr) {
    cpuflags_t flags = irq_save();
    asm volatile("invlpg (%0)" : : "r"(virt_addr) : "memory");
    irq_restore(flags);
}

int alloc_virtual_page(u64 virt_addr) {
    ssize_t addr = alloc_page();
    if (addr < 0)
        return -1;

    map_virtual_page(addr, virt_addr);

    return 0;
}

int alloc_virtual_pages(u64 virt_addr, size_t page_count) {
    for (; page_count; --page_count, virt_addr += PAGE_SIZE) {
        if (alloc_virtual_page(virt_addr))
            return -1;
    }

    return 0;
}

void free_virtual_page(u64 virt_addr) {
    struct pt_entry *pt_entry = get_page_entry(virt_addr);
    if (pt_entry != NULL && pt_entry->present) {
        free_page(pt_entry->addr);
        unmap_virtual_page(virt_addr);
    }
}

void free_virtual_pages(u64 virt_addr, size_t page_count) {
    for (; page_count; --page_count, virt_addr += PAGE_SIZE)
        free_virtual_page(virt_addr);
}
