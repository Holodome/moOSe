#include <vmalloc.h>
#include <arch/amd64/physmem.h>
#include <arch/amd64/virtmem.h>
#include <errno.h>
#include <kstdio.h>

static void *pbrk = VMALLOC_BASE;
static void *plimit = VMALLOC_BASE;

void *vsbrk(ssize_t increment) {
    void *prev_pbrk = pbrk;
    if (pbrk + increment < VMALLOC_BASE ||
        pbrk + increment > VMALLOC_LIMIT)
        return VSBRK_ERROR;

    if (pbrk + increment > plimit) {
        size_t alloc_size = pbrk + increment - plimit;
        size_t alloc_page_count = alloc_size / PAGE_SIZE;
        if (alloc_size % PAGE_SIZE != 0)
            alloc_page_count++;

        while (alloc_page_count--) {
            if (alloc_virtual_page((u64)plimit)) {
                errno = ENOMEM;
                return VSBRK_ERROR;
            }
            plimit += PAGE_SIZE;
        }
    }

    pbrk += increment;

    size_t free_page_count = (plimit - pbrk) / PAGE_SIZE;
    while (free_page_count--) {
        plimit -= PAGE_SIZE;
        free_virtual_page((u64)plimit);
    }

    kprintf("limit = %p\n", plimit);

    return prev_pbrk;
}
