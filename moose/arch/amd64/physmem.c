#include <arch/amd64/physmem.h>
#include <arch/amd64/memory_map.h>
#include <kstdio.h>

#define ARENA_SIZE (4 * 4096)

struct arena_allocator {
    char arena[ARENA_SIZE];
    u32 cursor;
};

static struct arena_allocator allocator;

static void init_allocator(void) {
    allocator.cursor = 0;
}

static void *alloc(size_t size) {
    if (allocator.cursor + size >= sizeof(allocator.arena))
        return NULL;

    allocator.cursor += size;
    return allocator.arena + allocator.cursor - size;
}

struct free_area {
    u8 order;
    u32 size;
    u64 *bitmap;
};

// 4k-512k page blocks
#define BUDDY_MAX_ORDER 8

// largest page block for buddy allocator
#define MAX_BLOCK_SIZE ((PAGE_SIZE << (BUDDY_MAX_ORDER - 1)))

// count of pages in the largest block
#define MAX_PAGE_COUNT (MAX_BLOCK_SIZE / PAGE_SIZE)

struct mem_zone {
    u64 mem_size;
    u32 used_page_count;
    u32 max_page_count;
    u64 base_addr;
    struct free_area free_area[BUDDY_MAX_ORDER];
};

struct phys_manager {
    struct mem_zone *zones;
};

static struct phys_manager phys_manager;

int init_phys_manager(void) {
    init_allocator();

    const struct memmap_entry *memmap;
    u32 memmap_size;

    get_memmap(&memmap, &memmap_size);

    u32 available_count = 0;
    for (u32 i = 0; i < memmap_size; ++i) {
        const struct memmap_entry *entry = memmap + i;
        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE)
            available_count++;
    }

    if (!available_count)
        return -1;

    phys_manager.zones = alloc(available_count * sizeof(struct mem_zone));

    for (u32 i = 0, zone_idx = 0; i < memmap_size; ++i) {
        const struct memmap_entry *entry = memmap + i;
        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            struct mem_zone *zone = &phys_manager.zones[zone_idx];
            zone->base_addr = entry->base;

            // align memory to the largest page block
            zone->mem_size = entry->length / MAX_BLOCK_SIZE * MAX_BLOCK_SIZE;
            zone->max_page_count = zone->mem_size / PAGE_SIZE;
            zone->used_page_count = 0;

            // count of the largest page blocks
            u32 block_count = zone->mem_size / MAX_BLOCK_SIZE;

            // alloc order bitmaps
            for (int j = BUDDY_MAX_ORDER-1; j >= 0; j--) {
                u32 bitmap_size = block_count * (1 << j) / 64;
                if (block_count * (1 << j) % 64 != 0)
                    bitmap_size++;

                u32 index = BUDDY_MAX_ORDER - j - 1;

                zone->free_area[index].bitmap = alloc(bitmap_size * 8);
                zone->free_area[index].size = block_count;
                zone->free_area[index].order = index;
            }

            zone_idx++;
        }
    }

    return 0;
}
