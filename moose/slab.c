#include <slab.h>
#include <param.h>
#include <mm/physmem.h>

#define FREE_QUEUE_END UINT_MAX

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

#define GENERAL_CACHE_COUNT 12
#define GENERAL_CACHE_MIN_SIZE 32
#define GENERAL_CACHE_MAX_SIZE 65536

// 1/8 of page size, if object size is larger than
// SLAB_THRESHOLD, slab descriptor will be put off slab
#define SLAB_THRESHOLD 512

static struct general_cache general_caches[] = {
    { "general-32",  32,    NULL},
    { "general-64",  64,    NULL},
    { "general-128", 128,   NULL},
    { "general-256", 256,   NULL},
    { "general-512", 512,   NULL},
    { "general-1k",  1024,  NULL},
    { "general-2k",  2048,  NULL},
    { "general-4k",  4096,  NULL},
    { "general-8k",  8192,  NULL},
    { "general-16k", 16384, NULL},
    { "general-32k", 32768, NULL},
    { "general-64k", 65536, NULL},
};

struct slab_cache *create_cache(const char *name, size_t size) {
    return NULL;
}

static int create_slab(struct slab_cache *cache) {
    ssize_t addr = alloc_pages(cache->order);
    if (addr < 0)
        return -1;

    struct slab *slab;
    if (cache->obj_size < SLAB_THRESHOLD) {
        slab = FIXUP_PTR((void *)addr);
        slab->used_count = 0;
        slab->free_queue = (u32 *)(slab + sizeof(struct slab));
        slab->memory = slab + sizeof(struct slab) +
                       cache->obj_count * sizeof(u32);

        for (u32 i = 0; i < cache->obj_count; i++)
            slab->free_queue[i] = i + 1;
        slab->free_queue[cache->obj_count - 1] = FREE_QUEUE_END;
    } else {
        // TODO: off slab descriptor allocation
    }

    list_add(&slab->list, &cache->slabs_free);

    return 0;
}

int init_slab_cache(void) {
    // TODO: cache_cache initialization
    if (create_slab(&cache_cache))
        return -1;

    list_add(&cache_cache.next, &caches);

    // TODO: init general caches
    for (u32 i = 0; i < GENERAL_CACHE_COUNT; i++) {
        struct slab_cache *cache = create_cache(
            general_caches[i].name, general_caches[i].obj_size);
        if (cache == NULL)
            return -1;

        general_caches[i].cache = cache;
        list_add(&cache->next, &caches);
    }

    return 0;
}
