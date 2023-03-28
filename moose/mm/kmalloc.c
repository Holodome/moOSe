#include <assert.h>
#include <bitops.h>
#include <list.h>
#include <mm/kmalloc.h>
#include <mm/vmalloc.h>
#include <param.h>
#include <string.h>

#define INITIAL_HEAP_SIZE (1 << 14)
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

static LIST_HEAD(subheaps);

static void init_subheap(struct subheap *heap) {
    init_list_head(&heap->blocks);
    init_list_head(&heap->list);

    struct mem_block *block = heap->memory;
    memset(block, 0, sizeof(*block));
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
    list_add(&initial_subheap.list, &subheaps);
}

static struct mem_block *subheap_find_best_block(struct subheap *heap,
                                                 size_t size) {
    struct mem_block *best_node = NULL;
    size_t best_mem = heap->size;

    struct mem_block *node;
    list_for_each_entry(node, &heap->blocks, list) {
        assert(node->size != 0);
        if (!node->used && node->size >= size && node->size < best_mem) {
            best_node = node;
            best_mem = node->size;
        }
    }

    return best_node;
}

static struct mem_block *find_best_block(size_t size) {
    struct subheap *heap;
    list_for_each_entry(heap, &subheaps, list) {
        struct mem_block *block = subheap_find_best_block(heap, size);
        if (block)
            return block;
    }

    return NULL;
}

static struct subheap *add_new_subheap(size_t min_size) {
    min_size += sizeof(struct mem_block) + sizeof(struct subheap);
    size_t size = align_po2(min_size, PAGE_SIZE);
    void *new_memory = vsbrk(size);
    if (!new_memory)
        return NULL;

    struct subheap *subheap = new_memory;
    subheap->memory = subheap + 1;
    subheap->size = size - sizeof(struct subheap);
    init_subheap(subheap);
    list_add(&subheap->list, &subheaps);
    return subheap;
}

void *kmalloc(size_t size) {
    if (size == 0)
        return NULL;

    size = align_po2(size, ALIGNMENT);
    struct mem_block *node = find_best_block(size);
    if (node == NULL) {
        struct subheap *heap = add_new_subheap(size);
        if (heap) {
            node = subheap_find_best_block(heap, size);
            assert(node);
        }
    }

    assert(size != 0);

    // OOM
    if (!node)
        return NULL;

    void *result = node + 1;
    if (node->size > size + sizeof(struct mem_block)) {
        struct mem_block *new_block = result + size;
        memset(new_block, 0, sizeof(*new_block));
        new_block->size = node->size - size - sizeof(struct mem_block);
        list_add(&new_block->list, &node->list);
        node->size = size;
        assert(node->size != 0);
        assert(new_block->size != 0);
    }

    node->used = 1;

    return result;
}

void *kzalloc(size_t size) {
    void *mem = kmalloc(size);
    memset(mem, 0, size);
    return mem;
}

static struct subheap *find_block_heap(struct mem_block *block) {
    uintptr_t block_addr = (uintptr_t)block;
    struct subheap *heap;
    list_for_each_entry(heap, &subheaps, list) {
        if (block_addr >= (uintptr_t)heap->memory &&
            block_addr < (uintptr_t)heap->memory + heap->size)
            return heap;
    }
    return NULL;
}

void kfree(void *mem) {
    if (mem == NULL)
        return;

    struct mem_block *block = (struct mem_block *)mem - 1;
    block->used = 0;
    struct subheap *subheap = find_block_heap(block);
    assert(subheap);

    struct mem_block *left =
        list_prev_entry_or_null(block, &subheap->blocks, list);
    if (left && !left->used) {
        left->size += block->size + sizeof(struct mem_block);
        list_remove(&block->list);
        block = left;
    }

    struct mem_block *right =
        list_next_entry_or_null(block, &subheap->blocks, list);
    if (right && !right->used) {
        block->size += right->size + sizeof(struct mem_block);
        list_remove(&right->list);
    }
}

char *kstrdup(const char *str) {
    if (str == NULL)
        return NULL;

    size_t len = strlen(str);
    void *memory = kmalloc(len + 1);
    strcpy(memory, str);
    return memory;
}

#ifndef __i386__
extern int kprintf(const char *, ...);
void print_malloc_info(void) {
    struct subheap *heap;
    struct mem_block *node;
    list_for_each_entry(heap, &subheaps, list) {
        kprintf("subheap %p-%p\n", heap->memory, heap->memory + heap->size);
        list_for_each_entry(node, &heap->blocks, list) {
            kprintf("block %p-%p\n", node + 1, node + 1 + node->size);
        }
    }
}
#endif
