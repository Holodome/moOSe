#include <kmalloc.h>
#include <kmem.h>
#include <kstdio.h>

#define TOTAL_MEM_SIZE 4096
#define ALIGNMENT 16

struct mem_block {
    size_t size;
    int used;
    struct list_head list;
} __attribute__((aligned(ALIGNMENT)));

static u8 memory[TOTAL_MEM_SIZE];
static LIST_HEAD(mem_start);

void init_memory(void) {
    struct mem_block *block = (void *)(memory);
    block->size = TOTAL_MEM_SIZE - sizeof(struct mem_block);
    list_add(&block->list, &mem_start);
}

static size_t calculate_size_to_alloc(size_t size) {
    size += sizeof(struct mem_block);
    size += ALIGNMENT - 1;
    size &= ~(ALIGNMENT - 1);
    return size;
}

static struct mem_block *best_fit(size_t size) {
    struct mem_block *best_node = NULL;
    size_t best_mem = TOTAL_MEM_SIZE + 1;

    list_for_each(iter, &mem_start) {
        struct mem_block *node = list_entry(iter, struct mem_block, list);
        if (!node->used &&
            (node->size == size ||
             node->size > size + sizeof(struct mem_block)) &&
            node->size < best_mem) {
            best_node = node;
            best_mem = node->size;
        }
    }

    kprintf("best fit result %zu %d\n", best_node->size, best_node->used);

    return best_node;
}

void *kmalloc(size_t size) {
    size_t size_to_alloc = calculate_size_to_alloc(size);
    struct mem_block *node = best_fit(size_to_alloc);
    if (node == NULL)
        return NULL;

    void *result = node + 1;
    // split block
    if (node->size > size_to_alloc + sizeof(struct mem_block)) {
        kprintf("here\n");
        struct mem_block *new_block = (void *)((char *)node + size_to_alloc);
        memset(new_block, 0, sizeof(*new_block));
        new_block->size = node->size - size_to_alloc - sizeof(struct mem_block);
        list_add(&new_block->list, &node->list);
        node->size = size_to_alloc;
    }

    node->used = 1;

    return result;
}

void *kzalloc(size_t size) {
    void *mem = kmalloc(size);
    memset(mem, 0, size);
    return mem;
}

void kfree(void *mem) {
    if (mem == NULL)
        return;

    struct mem_block *block = (struct mem_block *)mem - 1;
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

#if 0
    kprintf("memory block list\n");
    list_for_each(iter, &mem_start) {
        struct mem_block *node = list_entry(iter, struct mem_block, list);
        kprintf("block %p %zub %du\n", (void *)node, node->size, node->used);
    }
#endif 
}
