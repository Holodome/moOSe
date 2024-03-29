#include <moose/arch/amd64/types.h>
#include <moose/assert.h>
#include <moose/bitops.h>
#include <moose/mm/kmalloc.h>
#include <moose/mm/physmem.h>
#include <moose/param.h>

struct free_area {
    u32 size;
    u64 *bitmap;
};

struct mem_zone {
    u64 mem_size;
    u64 base_addr;
    struct free_area free_area[MAX_ORDER + 1];
};

static struct phys_mem {
    struct mem_zone *zones;
    u32 zone_count;
} phys_mem;

static void free_phys_mem(void) {
    for (size_t i = 0; i < phys_mem.zone_count; ++i) {
        struct mem_zone *zone = &phys_mem.zones[i];
        for (u32 j = 0; j <= MAX_ORDER; j++)
            kfree(zone->free_area[j].bitmap);
    }
    kfree(phys_mem.zones);
}

int init_phys_mem(const struct mem_range *ranges, size_t range_count) {
    phys_mem.zones = kzalloc(range_count * sizeof(*phys_mem.zones));
    if (phys_mem.zones == NULL)
        return -1;

    phys_mem.zone_count = range_count;
    for (u32 i = 0; i < range_count; ++i) {
        const struct mem_range *entry = ranges + i;
        struct mem_zone *zone = phys_mem.zones + i;
        zone->base_addr = entry->base;

        // count of the largest page blocks
        u32 block_count = entry->size >> MAX_BLOCK_SIZE_BITS;
        u32 max_order = MAX_ORDER;
        zone->mem_size = entry->size & ~(MAX_BLOCK_SIZE - 1);

        // if zone size less than max block size
        // buddy max order decreases
        if (entry->size < MAX_BLOCK_SIZE) {
            max_order = __log2(entry->size >> PAGE_SIZE_BITS);
            block_count = 1;
            zone->mem_size = block_count << PAGE_SIZE_BITS;
        }

        // alloc per order bitmaps
        for (int j = max_order; j >= 0; j--) {
            u32 bitmap_size = bits_to_bitmap(block_count << j);
            u64 *bitmap = kzalloc(bitmap_size * sizeof(u64));
            if (bitmap == NULL) {
                free_phys_mem();
                return -1;
            }

            u32 index = max_order - j;
            zone->free_area[index].size = block_count << j;
            zone->free_area[index].bitmap = bitmap;
        }
    }

    return 0;
}

static int test_buddies(const struct free_area *free_area, u32 order,
                        u64 index) {
    for (u32 i = 0; i <= order; i++) {
        u32 bit_count = 1 << i;
        u32 area_index = order - i;
        for (u32 j = 0; j < bit_count; j++)
            if (test_bit((index << i) + j, free_area[area_index].bitmap))
                return 1;
    }

    return 0;
}

static void set_buddies(struct free_area *free_area, u32 order, u64 index) {
    for (u32 i = 0; i <= order; i++) {
        u32 bit_count = 1 << i;
        u32 area_index = order - i;
        for (u32 j = 0; j < bit_count; j++)
            set_bit((index << i) + j, free_area[area_index].bitmap);
    }
}

static void clear_buddies(struct free_area *free_area, u32 order, u64 index) {
    for (u32 i = 0; i <= order; i++) {
        u32 bit_count = 1 << i;
        u32 area_index = order - i;
        for (u32 j = 0; j < bit_count; j++)
            clear_bit((index << i) + j, free_area[area_index].bitmap);
    }
}

ssize_t alloc_pages(u32 order) {
    assert(order <= MAX_ORDER);
    // finds first available memory zone
    for (u32 zone_idx = 0; zone_idx < phys_mem.zone_count; zone_idx++) {
        struct mem_zone *zone = &phys_mem.zones[zone_idx];
        struct free_area *area = &zone->free_area[order];
        if (area->bitmap == NULL)
            continue;

        for (u64 bit_idx = 0; bit_idx < area->size; bit_idx++) {
            if (!test_buddies(zone->free_area, order, bit_idx)) {
                set_buddies(zone->free_area, order, bit_idx);
                return zone->base_addr + (bit_idx << (order + PAGE_SIZE_BITS));
            }
        }
    }

    return -1;
}

void free_pages(u64 addr, u32 order) {
    assert((addr & 0xfff) == 0);
    assert(order <= MAX_ORDER);
    for (u32 zone_idx = 0; zone_idx < phys_mem.zone_count; zone_idx++) {
        struct mem_zone *zone = &phys_mem.zones[zone_idx];
        if (addr >= zone->base_addr &&
            addr < zone->base_addr + zone->mem_size) {
            u64 bit_idx = (addr - zone->base_addr) >> (order + PAGE_SIZE_BITS);
            clear_buddies(zone->free_area, order, bit_idx);
        }
    }
}

ssize_t alloc_page(void) {
    return alloc_pages(0);
}

void free_page(u64 addr) {
    free_pages(addr, 0);
}

int alloc_region(u64 addr, u64 count) {
    assert((addr & 0xfff) == 0);
    for (u32 zone_idx = 0; zone_idx < phys_mem.zone_count; zone_idx++) {
        struct mem_zone *zone = &phys_mem.zones[zone_idx];
        if (addr < zone->base_addr || addr >= zone->base_addr + zone->mem_size)
            continue;

        struct free_area *area = &zone->free_area[0];
        u64 bit_idx = (addr - zone->base_addr) >> PAGE_SIZE_BITS;
        if (bit_idx + count > area->size)
            return -1;

        for (u64 idx = bit_idx; idx < bit_idx + count; idx++)
            if (test_bit(idx, area->bitmap))
                return -1;

        for (u64 idx = bit_idx; idx < bit_idx + count; idx++)
            set_bit(idx, area->bitmap);
    }

    return 0;
}

void free_region(u64 addr, u64 count) {
    for (u64 i = 0; i < count; i++)
        free_page(addr);
}
