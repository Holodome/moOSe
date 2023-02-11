#pragma once

#include <types.h>
#include <list.h>

#define CACHE_NAME_SIZE 32

struct slab_cache {
    struct list_head slabs_full;
    struct list_head slabs_partial;
    struct list_head slabs_free;
    u32 obj_size;
    u32 obj_count;
    u32 order;
    char name[CACHE_NAME_SIZE];
    struct list_head next;
    // TODO: add constructor and destructor
    u32 active_count;
    u32 allocated_count;
};

struct slab {
    struct list_head list;
    void *memory;
    u32 *free_queue;
    u32 used_count;
};

int init_slab_cache(void);

struct slab_cache *create_cache(const char *name, size_t size);
