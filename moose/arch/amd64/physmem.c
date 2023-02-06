#include <arch/amd64/memory_map.h>
#include <arch/amd64/physmem.h>
#include <bitops.h>
#include <kmalloc.h>
#include <kstdio.h>

// 4k-512k page blocks
#define BUDDY_MAX_ORDER 7
#define MAX_BLOCK_SIZE_LOG (PAGE_SIZE_LOG + BUDDY_MAX_ORDER)

// largest page block for buddy allocator
#define MAX_BLOCK_SIZE ((PAGE_SIZE << (BUDDY_MAX_ORDER)))

struct free_area {
    u32 size;
    u64 *bitmap;
};

struct mem_zone {
    u64 mem_size;
    u32 used_page_count;
    u32 max_page_count;
    u64 base_addr;
    struct free_area free_area[BUDDY_MAX_ORDER + 1];
};

struct phys_mem {
    struct mem_zone *zones;
    u32 zones_size;
};

static struct phys_mem phys_mem;

int init_phys_mem(const struct memmap_entry *memmap, size_t memmap_size) {
    u32 available_count = 0;
    for (u32 i = 0; i < memmap_size; ++i) {
        const struct memmap_entry *entry = memmap + i;
        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE)
            available_count++;
    }

    if (!available_count)
        return -1;

    phys_mem.zones = kzalloc(available_count * sizeof(*phys_mem.zones));
    if (phys_mem.zones == NULL)
        return -1;

    phys_mem.zones_size = available_count;
    for (u32 i = 0, zone_idx = 0; i < memmap_size; ++i) {
        const struct memmap_entry *entry = memmap + i;
        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            struct mem_zone *zone = phys_mem.zones + zone_idx++;
            zone->base_addr = entry->base;
            zone->mem_size = align_po2(entry->length, MAX_BLOCK_SIZE);
            zone->max_page_count = zone->mem_size >> PAGE_SIZE_LOG;

            u32 block_count = zone->mem_size >> MAX_BLOCK_SIZE_LOG;
            for (int j = BUDDY_MAX_ORDER; j >= 0; j--) {
                u32 bitmap_size = bits_to_bitmap(block_count << j);
                if ((block_count << j) & 0x3f)
                    bitmap_size++;

                u32 index = BUDDY_MAX_ORDER - j;

                void *bitmap = kmalloc(bitmap_size * sizeof(u64));
                if (bitmap == NULL)
                    goto free_memzones;

                zone->free_area[index].bitmap = bitmap;
                zone->free_area[index].size = block_count * (1 << j);
            }
        }
    }

    return 0;
free_memzones:
    for (size_t i = 0; i < phys_mem.zones_size; ++i) {
        struct mem_zone *zone = &phys_mem.zones[i];
        for (int j = BUDDY_MAX_ORDER; j >= 0; j--)
            kfree(zone->free_area[j].bitmap);
    }
    kfree(phys_mem.zones);
    return -1;
}

static u64 test_buddies(struct free_area *free_area, u32 buddy, u64 index,
                        size_t count) {
    for (int i = buddy; i >= 0; i--) {
        u32 offset = buddy - i;
        u32 bit_count = count >> i;
        if (count & ((1u << i) - 1))
            bit_count++;

        for (u32 j = 0; j < bit_count; j++) {
            if (test_bit(free_area[i].bitmap, (index << offset) + j))
                return 1;
        }
    }

    return 0;
}

static void set_buddies(struct free_area *free_area, u32 buddy, u64 index,
                        size_t count) {
    for (int i = buddy; i >= 0; i--) {
        u32 offset = buddy - i;
        u32 bit_count = count >> i;
        if (count & ((1u << i) - 1))
            bit_count++;

        for (u32 j = 0; j < bit_count; j++)
            set_bit(free_area[i].bitmap, (index << offset) + j);
    }
}

static void unset_buddies(struct free_area *free_area, u32 buddy, u64 index,
                          size_t count) {
    for (int i = buddy; i >= 0; i--) {
        u32 offset = buddy - i;
        u32 bit_count = count >> i;
        if (count % (1u << i) != 0)
            bit_count++;

        for (u32 j = 0; j < bit_count; j++)
            clear_bit(free_area[i].bitmap, (index << offset) + j);
    }
}

ssize_t alloc_pages(size_t count) {
    u32 order = bit_scan_reverse(count);
    if (order > BUDDY_MAX_ORDER)
        order = BUDDY_MAX_ORDER;

    // finds first available memory zone
    for (u32 zone_idx = 0; zone_idx < phys_mem.zones_size; zone_idx++) {
        struct mem_zone *zone = &phys_mem.zones[zone_idx];
        struct free_area *area = &zone->free_area[order];

        if (zone->used_page_count + count > zone->max_page_count)
            continue;

        for (u64 bit_idx = 0; bit_idx < area->size; bit_idx++) {
            if (!test_buddies(zone->free_area, order, bit_idx, count)) {
                set_buddies(zone->free_area, order, bit_idx, count);
                zone->used_page_count += count;

                return (zone->base_addr + (bit_idx << order)) << PAGE_SIZE_LOG;
            }
        }
    }

    return -1;
}

void free_pages(u64 addr, size_t count) {
    if (addr % PAGE_SIZE != 0)
        return;

    u32 order = bit_scan_reverse(count);
    if (order > BUDDY_MAX_ORDER)
        order = BUDDY_MAX_ORDER;

    for (u32 zone_idx = 0; zone_idx < phys_mem.zones_size; zone_idx++) {
        struct mem_zone *zone = &phys_mem.zones[zone_idx];
        if (addr >= zone->base_addr &&
            addr < zone->base_addr + zone->mem_size) {

            u64 bit_idx = (addr - zone->base_addr) >> (order + PAGE_SIZE_LOG);

            unset_buddies(zone->free_area, order, bit_idx, count);
            zone->used_page_count -= count;
        }
    }
}

ssize_t alloc_page(void) { return alloc_pages(1); }

void free_page(u64 addr) { free_pages(addr, 1); }

ssize_t alloc_region(u64 addr, size_t count) {
    // base addr must be aligned by page size
    if (addr % PAGE_SIZE != 0)
        return -1;

    u32 order = bit_scan_reverse(count);
    if (order > BUDDY_MAX_ORDER)
        order = BUDDY_MAX_ORDER;

    for (u32 zone_idx = 0; zone_idx < phys_mem.zones_size; zone_idx++) {
        struct mem_zone *zone = &phys_mem.zones[zone_idx];

        if (addr >= zone->base_addr &&
            addr < zone->base_addr + zone->mem_size &&
            zone->base_addr + zone->mem_size >
                addr + (count << PAGE_SIZE_LOG)) {
            u64 bit_idx = (addr - zone->base_addr) >> (order + PAGE_SIZE_LOG);
            if (!test_buddies(zone->free_area, order, bit_idx, count)) {
                set_buddies(zone->free_area, order, bit_idx, count);
                zone->used_page_count += count;

                return addr;
            }
        }
    }

    return -1;
}

void free_region(u64 addr, size_t count) { free_pages(addr, count); }
