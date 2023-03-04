#include <slab.h>
#include <mm/kmem.h>
#include <bitops.h>
#include <param.h>
#include <mm/physmem.h>

#define FREE_QUEUE_END UINT_MAX
#define FREE_QUEUE_PTR(_slab) \
    ((u32 *)(((struct slab*)_slab)+1))

#define ALIGNMENT 16

// cache chain
static LIST_HEAD(caches);

static struct slab_cache cache_cache = {
    .slabs_full = LIST_HEAD_INIT(cache_cache.slabs_full),
    .slabs_partial = LIST_HEAD_INIT(cache_cache.slabs_partial),
    .slabs_free = LIST_HEAD_INIT(cache_cache.slabs_free),
    .obj_size = sizeof(struct slab_cache),
    .name = "slab_cache"
};

struct general_cache {
    char *name;
    size_t obj_size;
    struct slab_cache *cache;
};

#define GENERAL_CACHE_COUNT 8
#define GENERAL_CACHE_MIN_SIZE 16
#define GENERAL_CACHE_MIN_SIZE_BITS 4
#define GENERAL_CACHE_MAX_SIZE 2048

static struct general_cache general_caches[] = {
    { "general-16",  16,    NULL},
    { "general-32",  32,    NULL},
    { "general-64",  64,    NULL},
    { "general-128", 128,   NULL},
    { "general-256", 256,   NULL},
    { "general-512", 512,   NULL},
    { "general-1k",  1024,  NULL},
    { "general-2k",  2048,  NULL},
};

static void estimate_cache(struct slab_cache *cache) {
    size_t mem_size = PAGE_SIZE;
    cache->obj_count = (mem_size - sizeof(struct slab)) /
                       (sizeof(u32) + cache->obj_size);

    cache->slab_header_size = align_po2(sizeof(struct slab) +
                                            cache->obj_count * sizeof(u32), ALIGNMENT);
    if (cache->slab_header_size +
            cache->obj_count * cache->obj_size > mem_size)
        cache->obj_count--;
}

struct slab_cache *create_cache(const char *name, size_t size) {
    // alloc cache from cache_cache
    struct slab_cache *cache = cache_alloc(&cache_cache);
    if (cache == NULL)
        return NULL;

    init_list_head(&cache->slabs_free);
    init_list_head(&cache->slabs_partial);
    init_list_head(&cache->slabs_full);

    strncpy(cache->name, name, CACHE_NAME_SIZE);
    cache->obj_size = align_po2(size, ALIGNMENT);

    estimate_cache(cache);

    cache->active_count = 0;
    if (grow_cache(cache))
        return NULL;

    list_add(&cache->list, &caches);

    return cache;
}

void free_cache(struct slab_cache *cache) {
    list_for_each(iter, &cache->slabs_full) {
        struct slab *slab = list_entry(iter, struct slab, list);
        free_page((u64)PTR_TO_PHYS(slab));
    }

    list_for_each(iter, &cache->slabs_partial) {
        struct slab *slab = list_entry(iter, struct slab, list);
        free_page((u64)PTR_TO_PHYS(slab));
    }

    list_for_each(iter, &cache->slabs_free) {
        struct slab *slab = list_entry(iter, struct slab, list);
        free_page((u64)PTR_TO_PHYS(slab));
    }

    cache_free(&cache_cache, cache);
}

int grow_cache(struct slab_cache *cache) {
    ssize_t addr = alloc_page();
    if (addr < 0)
        return -1;

    struct slab *slab = FIXUP_PTR((void *)addr);
    slab->memory = (void *)slab + sizeof(struct slab) +
                   cache->obj_count * sizeof(u32);

    slab->cache = cache;

    slab->used_count = 0;
    u32 *free_queue = FREE_QUEUE_PTR(slab);
    for (u32 i = 0; i < cache->obj_count; i++)
        free_queue[i] = i + 1;
    free_queue[cache->obj_count - 1] = FREE_QUEUE_END;
    slab->free = 0;

    list_add(&slab->list, &cache->slabs_free);
    cache->allocated_count += cache->obj_count;

    return 0;
}

void shrink_cache(struct slab_cache *cache) {
    list_for_each(iter, &cache->slabs_free) {
        struct slab *slab = list_entry(iter, struct slab, list);
        free_page((u64)PTR_TO_PHYS(slab));
    }
}

void *cache_alloc(struct slab_cache *cache) {
    struct list_head *partial = &cache->slabs_partial;
    struct list_head *free = &cache->slabs_free;

    struct slab *slab = list_next_or_null(partial, partial, struct slab, list);
    if (slab == NULL) {
        slab = list_next_or_null(free, free, struct slab, list);
        if (slab == NULL) {
            if (grow_cache(cache))
                return NULL;
            slab = list_next_or_null(free, free, struct slab, list);
        }
    }

    if (slab->used_count == 0) {
        list_remove(&slab->list);
        list_add(&slab->list, partial);
    }

    void *obj = slab->memory + slab->free * cache->obj_size;
    slab->free = FREE_QUEUE_PTR(slab)[slab->free];

    slab->used_count++;
    if (slab->used_count == cache->obj_count) {
        list_remove(&slab->list);
        list_add(&slab->list, &cache->slabs_full);
    }

    return obj;
}

void cache_free(struct slab_cache *cache, void *obj) {
    struct slab *slab = (void *)((u64)obj & ~0xfff);

    u32 index = (obj - slab->memory) / cache->obj_size;
    FREE_QUEUE_PTR(slab)[index] = slab->free;
    slab->free = index;

    if (slab->used_count == cache->obj_count) {
        list_remove(&slab->list);
        list_add(&slab->list, &cache->slabs_partial);
    }

    slab->used_count--;
    if (slab->used_count == 0) {
        list_remove(&slab->list);
        list_add(&slab->list, &cache->slabs_free);
    }
}

void *smalloc(size_t size) {
    size = align_po2(size, GENERAL_CACHE_MIN_SIZE);
    if (size > GENERAL_CACHE_MAX_SIZE)
        return NULL;

    size_t aligned = 1 << __log2(size);
    if (aligned != size)
        aligned <<= 1;

    u32 index = __log2(aligned >> GENERAL_CACHE_MIN_SIZE_BITS);

    return cache_alloc(general_caches[index].cache);
}

void sfree(void *ptr) {
    struct slab *slab = (void *)((u64)ptr & ~0xfff);
    cache_free(slab->cache, ptr);
}

int init_slab_cache(void) {
    // start initialization of cache_cache
    // it is the first available cache in chain
    cache_cache.obj_size = align_po2(cache_cache.obj_size, ALIGNMENT);
    estimate_cache(&cache_cache);
    if (grow_cache(&cache_cache))
        return -1;

    list_add(&cache_cache.list, &caches);

    // init general caches for smalloc, sfree
    for (u32 i = 0; i < GENERAL_CACHE_COUNT; i++) {
        struct slab_cache *cache = create_cache(
            general_caches[i].name, general_caches[i].obj_size);
        if (cache == NULL)
            return -1;

        general_caches[i].cache = cache;
        list_add(&cache->list, &caches);
    }

    return 0;
}
