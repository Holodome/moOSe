#include <arch/amd64/virtmem.h>
#include <assert.h>
#include <bitops.h>
#include <list.h>
#include <mm/kmalloc.h>
#include <param.h>
#include <sched/locks.h>
#include <string.h>

#define BRK_BASE ((uintptr_t)0xffffc90000000000llu)
#define BRK_LIMIT ((uintptr_t)0xffffe8ffffffffffllu)

#define INITIAL_HEAP_SIZE (1 << 19)
#define ALIGNMENT 16

struct mem_block {
    size_t size;
    int used;
    struct list_head list;
} __aligned(ALIGNMENT);

struct subheap {
    void *memory;
    size_t size;

    struct list_head blocks;
    struct list_head list;
} __aligned(ALIGNMENT);

static struct {
    struct list_head subheaps;
    spinlock_t lock;
    uintptr_t pbrk;
    uintptr_t plimit;
} kmalloc_state = {.subheaps = INIT_LIST_HEAD(kmalloc_state.subheaps),
                   .lock = INIT_SPIN_LOCK(),
                   .pbrk = BRK_BASE,
                   .plimit = BRK_BASE};

// expand-only version of sbrk
static void *sbrk(intptr_t increment) {
    uintptr_t pbrk = kmalloc_state.pbrk;
    uintptr_t plimit = kmalloc_state.plimit;
    uintptr_t prev_pbrk = pbrk;
    if (pbrk + increment < BRK_BASE || pbrk + increment > BRK_LIMIT)
        return NULL;

    if (pbrk + increment > plimit) {
        size_t alloc_size = pbrk + increment - plimit;
        size_t alloc_page_count =
            align_po2(alloc_size, PAGE_SIZE) >> PAGE_SIZE_BITS;

        if (alloc_virtual_pages(plimit, alloc_page_count))
            return NULL;

        plimit += alloc_page_count << PAGE_SIZE_BITS;
    }

    pbrk += increment;
    kmalloc_state.pbrk = pbrk;
    kmalloc_state.plimit = plimit;

    return (void *)prev_pbrk;
}

static void init_subheap(struct subheap *heap) {
    init_list_head(&heap->blocks);
    struct mem_block *block = heap->memory;
    block->size = heap->size - sizeof(struct mem_block);
    list_add(&block->list, &heap->blocks);
}

void init_kmalloc(void) {
    static u8 initial_memory[INITIAL_HEAP_SIZE];
    static struct subheap initial_subheap = {
        .memory = initial_memory,
        .size = INITIAL_HEAP_SIZE,
    };
    init_subheap(&initial_subheap);
    list_add(&initial_subheap.list, &kmalloc_state.subheaps);
}

static struct mem_block *subheap_find_best_block(struct subheap *heap,
                                                 size_t size) {
    struct mem_block *best_node = NULL;
    size_t best_mem = heap->size;

    struct mem_block *node;
    list_for_each_entry(node, &heap->blocks, list) {
        expects(node->size != 0);
        if (!node->used && node->size >= size && node->size < best_mem) {
            best_node = node;
            best_mem = node->size;
        }
    }

    return best_node;
}

static struct mem_block *find_best_block(size_t size) {
    struct subheap *heap;
    list_for_each_entry(heap, &kmalloc_state.subheaps, list) {
        struct mem_block *block = subheap_find_best_block(heap, size);
        if (block)
            return block;
    }

    return NULL;
}

static struct subheap *add_new_subheap(size_t min_size) {
    min_size += sizeof(struct mem_block) + sizeof(struct subheap);
    size_t size = align_po2(min_size, PAGE_SIZE);
    void *new_memory = sbrk(size);
    if (!new_memory)
        return NULL;

    struct subheap *subheap = new_memory;
    subheap->memory = subheap + 1;
    subheap->size = size - sizeof(struct subheap);
    init_subheap(subheap);
    list_add(&subheap->list, &kmalloc_state.subheaps);
    return subheap;
}

void *kmalloc(size_t size) {
    if (size == 0)
        return NULL;

    size = align_po2(size, ALIGNMENT);
    expects(size != 0);

    cpuflags_t flags = spin_lock_irqsave(&kmalloc_state.lock);
    struct mem_block *node = find_best_block(size);
    if (node == NULL) {
        struct subheap *heap = add_new_subheap(size);
        if (heap) {
            node = subheap_find_best_block(heap, size);
            expects(node);
        }
    }

    // OOM
    if (!node) {
        spin_unlock_irqrestore(&kmalloc_state.lock, flags);
        return NULL;
    }

    void *result = node + 1;
    if (node->size > size + sizeof(struct mem_block)) {
        struct mem_block *new_block = result + size;
        memset(new_block, 0, sizeof(*new_block));
        new_block->size = node->size - size - sizeof(struct mem_block);
        list_add(&new_block->list, &node->list);
        node->size = size;
        expects(node->size != 0);
        expects(new_block->size != 0);
    }

    node->used = 1;
    spin_unlock_irqrestore(&kmalloc_state.lock, flags);

    return result;
}

void *kzalloc(size_t size) {
    void *mem = kmalloc(size);
    if (mem)
        memset(mem, 0, size);

    return mem;
}

static struct subheap *find_block_heap(struct mem_block *block) {
    uintptr_t block_addr = (uintptr_t)block;
    struct subheap *heap;
    list_for_each_entry(heap, &kmalloc_state.subheaps, list) {
        if (block_addr >= (uintptr_t)heap->memory &&
            block_addr < (uintptr_t)heap->memory + heap->size)
            return heap;
    }
    return NULL;
}

void kfree(void *mem) {
    if (mem == NULL)
        return;

    cpuflags_t flags = spin_lock_irqsave(&kmalloc_state.lock);

    struct mem_block *block = (struct mem_block *)mem - 1;
    block->used = 0;
    struct subheap *subheap = find_block_heap(block);
    assert(subheap);

    struct mem_block *left =
        list_prev_entry_or_null(block, &subheap->blocks, list);
    expects(left->size);
    if (left && !left->used) {
        left->size += block->size + sizeof(struct mem_block);
        list_remove(&block->list);
        block = left;
    }

    struct mem_block *right =
        list_next_entry_or_null(block, &subheap->blocks, list);
    expects(right->size);
    if (right && !right->used) {
        block->size += right->size + sizeof(struct mem_block);
        list_remove(&right->list);
    }

    spin_unlock_irqrestore(&kmalloc_state.lock, flags);
}

char *kstrdup(const char *str) {
    if (str == NULL)
        return NULL;

    size_t len = strlen(str);
    void *memory = kmalloc(len + 1);
    if (memory)
        strcpy(memory, str);

    return memory;
}
