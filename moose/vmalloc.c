#include <vmalloc.h>
#include <kernel.h>
#include <arch/amd64/virtmem.h>
#include <errno.h>
#include <types.h>
#include <kmem.h>
#include <kstdio.h>

static void *pbrk = VMALLOC_BASE;
static void *plimit = VMALLOC_BASE;

int vbrk(void *addr) {
    if (addr < VMALLOC_BASE || addr > VMALLOC_LIMIT)
        return 1;

    return vsbrk(addr - pbrk) == NULL;
}

void *vsbrk(ssize_t increment) {
    void *prev_pbrk = pbrk;
    if (pbrk + increment < VMALLOC_BASE ||
        pbrk + increment > VMALLOC_LIMIT)
        return NULL;

    if (pbrk + increment > plimit) {
        size_t alloc_size = pbrk + increment - plimit;
        size_t alloc_page_count = alloc_size / PAGE_SIZE;
        if (alloc_size % PAGE_SIZE != 0)
            alloc_page_count++;

        while (alloc_page_count--) {
            if (alloc_virtual_page((u64)plimit)) {
                errno = ENOMEM;
                return NULL;
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

    return prev_pbrk;
}

#define ALIGNMENT 16

struct mem_block {
    size_t size;
    int used;
    struct list_head list;
} __attribute__((aligned(ALIGNMENT)));

static size_t align_alloc_size(size_t size) {
    size += ALIGNMENT - 1;
    size &= ~(ALIGNMENT - 1);
    return size;
}

static LIST_HEAD(mem_start);

static struct mem_block *next_free_block(size_t size) {
    struct mem_block *best_block = NULL;
    size_t best_fit_size = ULLONG_MAX;

    list_for_each(iter, &mem_start) {
        struct mem_block *block = list_entry(iter, struct mem_block, list);
        if (!block->used && block->size >= size && block->size < best_fit_size) {
            best_block = block;
            best_fit_size = block->size;
        }
    }

    return best_block;
}

void *vmalloc(size_t size) {
    size = align_alloc_size(size);

    struct mem_block *free_block = next_free_block(size);
    if (free_block == NULL) {
        size_t sbrk_size = ((size + sizeof(struct mem_block))
                                / PAGE_SIZE + 1) * PAGE_SIZE;
        struct mem_block *new_block = vsbrk(sbrk_size);
        if (new_block == NULL)
            return NULL;

        new_block->size = sbrk_size - sizeof(struct mem_block);
        list_add(&new_block->list, &mem_start);
        free_block = new_block;
    }

    void *result = free_block + 1;
    if (free_block->size > size + sizeof(struct mem_block)) {
        struct mem_block *new_block = (void *)((char *)result + size);
        memset(new_block, 0, sizeof(struct mem_block));
        new_block->size = free_block->size - size - sizeof(struct mem_block);
        list_add(&new_block->list, &free_block->list);
        free_block->size = size;
    }

    free_block->used = 1;

    return result;
}

void vfree(const void *addr) {
    if (addr == NULL)
        return;

    struct mem_block *block = (struct mem_block *)addr - 1;
    block->used = 0;

    struct mem_block *left =
        list_prev_or_null(&block->list, &mem_start, struct mem_block, list);
    if (left && !left->used) {
        left->size += block->size + sizeof(struct mem_block);
        list_remove(&block->list);
        block = left;
    }

    struct mem_block *right =
        list_next_or_null(&block->list, &mem_start, struct mem_block, list);
    if (right && !right->used) {
        block->size += right->size + sizeof(struct mem_block);
        list_remove(&right->list);
    }
}
