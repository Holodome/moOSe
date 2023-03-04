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
    char name[CACHE_NAME_SIZE];
    struct list_head list;
    // TODO: add constructor and destructor
    u32 active_count;
    u32 allocated_count;
    u32 slab_header_size;
};

struct slab {
    struct list_head list;
    struct slab_cache *cache;
    void *memory;
    u32 used_count;
    u32 free;
};

int init_slab_cache(void);

struct slab_cache *create_cache(const char *name, size_t size);
void free_cache(struct slab_cache *cache);
int grow_cache(struct slab_cache *cache);
void shrink_cache(struct slab_cache *cache);

void *cache_alloc(struct slab_cache *cache);
void cache_free(struct slab_cache *cache, void *obj);

void *smalloc(size_t size);
void sfree(void *ptr);
