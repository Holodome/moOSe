#include <arch/amd64/virtmem.h>
#include <bitops.h>
#include <mm/vmalloc.h>
#include <param.h>
#include <types.h>

#define VMALLOC_BASE ((uintptr_t)0xffffc90000000000llu)
#define VMALLOC_LIMIT ((uintptr_t)0xffffe8ffffffffffllu)

static uintptr_t pbrk = VMALLOC_BASE;
static uintptr_t plimit = VMALLOC_BASE;

int vbrk(void *addr_) {
    uintptr_t addr = (uintptr_t)addr_;
    if (addr < VMALLOC_BASE || addr > VMALLOC_LIMIT)
        return -1;

    return vsbrk(addr - pbrk) == NULL;
}

void *vsbrk(intptr_t increment) {
    uintptr_t prev_pbrk = pbrk;
    if (pbrk + increment < VMALLOC_BASE || pbrk + increment > VMALLOC_LIMIT)
        return NULL;

    if (pbrk + increment > plimit) {
        size_t alloc_size = pbrk + increment - plimit;
        size_t alloc_page_count =
            align_po2(alloc_size, PAGE_SIZE) >> PAGE_SIZE_BITS;

        // TODO: Library call for mapping virt region
        while (alloc_page_count--) {
            if (alloc_virtual_page((u64)plimit))
                return NULL;

            plimit += PAGE_SIZE;
        }
    }

    pbrk += increment;

    size_t free_page_count = (plimit - pbrk) >> PAGE_SIZE_BITS;
    while (free_page_count--) {
        plimit -= PAGE_SIZE;
        free_virtual_page((u64)plimit);
    }

    return (void *)prev_pbrk;
}
