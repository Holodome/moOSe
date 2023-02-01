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
    u32 zones_size;
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
    phys_manager.zones_size = available_count;

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
                zone->free_area[index].size = block_count * (1 << j);
                zone->free_area[index].order = index;
            }

            zone_idx++;
        }
    }

    return 0;
}

static const int tab32[32] = {
    0,  9,  1, 10, 13, 21,  2, 29,
    11, 14, 16, 18, 22, 25,  3, 30,
    8, 12, 20, 28, 15, 17, 24,  7,
    19, 27, 23,  6, 26,  5,  4, 31};

static u32 log2_32(u32 value)
{
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return tab32[(u32) (value * 0x07C4ACDD) >> 27];
}

static inline u64 test_bit(const u64 *bitmap, u64 index) {
    return bitmap[index / 64] & (1l << (index % 64));
}

static inline void set_bit(u64 *bitmap, u64 index) {
    bitmap[index / 64] |= (1l << (index % 64));
}

static inline void unset_bit(u64 *bitmap, u64 index) {
    bitmap[index / 64] &= ~(1 << (index % 64));
}

static u64 test_buddies(struct free_area *free_area, u32 buddy, u64 index) {
    for (int i = buddy; i >= 0; i--) {
        u32 offset = buddy - i;
        for (int j = 0; j < (1l << offset); j++) {
            if (test_bit(free_area[i].bitmap, index * (1l << offset) + j))
                return 1;
        }
    }

    return 0;
}

static void set_buddies(struct free_area *free_area, u32 buddy, u64 index) {
    for (int i = buddy; i >= 0; i--) {
        u32 offset = buddy - i;
        for (u32 j = 0; j < ((u64) 1 << offset); j++)
            set_bit(free_area[i].bitmap, index * (1l << offset) + j);
    }
}

//static void unset_buddies(struct free_area *free_area, u32 buddy, u64 index) {
//    for (int i = buddy; i >= 0; i--) {
//        u32 offset = buddy - i;
//        for (int j = 0; j < (1l << offset); j++)
//            unset_bit(free_area[i].bitmap, index * (1l << offset) + j);
//    }
//}

struct page_block *alloc_page_block(size_t count) {
    u32 order = log2_32(count);
    if (count != 1u << order)
        order++;

    if (order >= BUDDY_MAX_ORDER)
        order = BUDDY_MAX_ORDER - 1;

    for (u32 zone_idx = 0; zone_idx < phys_manager.zones_size; zone_idx++) {
        struct mem_zone *zone = &phys_manager.zones[zone_idx];
        struct free_area *area = &zone->free_area[order];

        for (u64 bit_idx = 0; bit_idx < area->size; bit_idx++) {
            if (!test_buddies(zone->free_area, order, bit_idx)) {
                set_buddies(zone->free_area, order, bit_idx);
                struct page_block *block = alloc(sizeof(struct page_block));

                block->count = count;
                block->addr = zone->base_addr +
                              bit_idx * (1l << order) * PAGE_SIZE;

                return block;
            }
        }
    }

    return NULL;
}

//void free_page_block(struct page_block *page) {
//
//}
