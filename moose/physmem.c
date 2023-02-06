#include <physmem.h>
#include <bitops.h>
#include <kmalloc.h>
#include <kmem.h>
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

// TODO: memory zones list as argument
int init_phys_mem(const struct mem_range *memmap, size_t memmap_size) {
    // TODO: Assert memmap_size != 0
    phys_mem.zones = kzalloc(memmap_size * sizeof(*phys_mem.zones));
    if (phys_mem.zones == NULL)
        return -1;

    phys_mem.zones_size = memmap_size;
    for (u32 i = 0, zone_idx = 0; i < memmap_size; ++i) {
        const struct mem_range *entry = memmap + i;
        struct mem_zone *zone = phys_mem.zones + zone_idx++;
        zone->base_addr = entry->base;
        zone->mem_size = align_po2(entry->size, MAX_BLOCK_SIZE);
        zone->max_page_count = zone->mem_size >> PAGE_SIZE_LOG;

        u32 block_count = zone->mem_size >> MAX_BLOCK_SIZE_LOG;
        for (int j = BUDDY_MAX_ORDER; j >= 0; j--) {
            u32 bitmap_size = bits_to_bitmap(block_count << j);
            u32 index = BUDDY_MAX_ORDER - j;

            void *bitmap = kmalloc(bitmap_size * sizeof(u64));
            if (bitmap == NULL)
                goto free_memzones;

            memset(bitmap, 0xff, bitmap_size * sizeof(u64));
            zone->free_area[index].bitmap = bitmap;
            zone->free_area[index].size = block_count * (1 << j);
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

static void set_buddies(struct mem_zone *zone, u64 order, u64 buddy_idx) {
    clear_bit(zone->free_area[order].bitmap, buddy_idx);
    for (int i = order - 1; i >= 0; --i)
        for (u64 idx = buddy_idx << (order - i);
             idx < (buddy_idx + 1) * (order - i); ++idx)
            clear_bit(zone->free_area[i].bitmap, idx);
}

static void clear_buddies(struct mem_zone *zone, u64 order, u64 buddy_idx) {
    set_bit(zone->free_area[order].bitmap, buddy_idx);
    for (int i = order - 1; i >= 0; --i)
        for (u64 idx = buddy_idx << (order - i);
             idx < (buddy_idx + 1) * (order - i); ++idx)
            clear_bit(zone->free_area[i].bitmap, idx);
}

ssize_t alloc_pages(size_t order) {
    for (u32 zone_idx = 0; zone_idx < phys_mem.zones_size; zone_idx++) {
        struct mem_zone *zone = &phys_mem.zones[zone_idx];
        struct free_area *area = &zone->free_area[order];

        if (zone->used_page_count + (1 << order) > zone->max_page_count)
            continue;

        u64 *bitmap = area->bitmap;
        for (u64 bit_idx = 0; bit_idx < area->size;
             bit_idx += CHAR_BIT * sizeof(u64), ++bitmap) {
            u32 found_bit = bit_scan_forward(*bitmap);
            if (found_bit == 0)
                continue;

            u64 buddy_idx = bit_idx + found_bit;
            set_buddies(zone, order, buddy_idx);

            zone->used_page_count += 1 << order;
            return zone->base_addr + (buddy_idx << (order + PAGE_SIZE_LOG));
        }
    }

    return -1;
}

void free_pages(u64 addr, size_t order) {
    // TODO: assert addr alignment
    for (u32 zone_idx = 0; zone_idx < phys_mem.zones_size; zone_idx++) {
        struct mem_zone *zone = &phys_mem.zones[zone_idx];
        if (addr >= zone->base_addr &&
            addr < zone->base_addr + zone->mem_size) {
            u64 buddy_idx = (addr - zone->base_addr) >> (order + PAGE_SIZE_LOG);
            clear_buddies(zone, order, buddy_idx);
            zone->used_page_count -= 1 << order;
        }
    }
}

ssize_t alloc_region(u64 addr, size_t order) {
    // TODO: assert addr alignment
    for (u32 zone_idx = 0; zone_idx < phys_mem.zones_size; zone_idx++) {
        struct mem_zone *zone = &phys_mem.zones[zone_idx];

        if (addr >= zone->base_addr &&
            addr < zone->base_addr + zone->mem_size &&
            zone->base_addr + zone->mem_size >
                addr + (1 << (PAGE_SIZE_LOG + order))) {
            u64 bit_idx = (addr - zone->base_addr) >> (order + PAGE_SIZE_LOG);
            if (test_bit(zone->free_area[order].bitmap, bit_idx))
                return -1;

            set_buddies(zone, order, bit_idx);
            zone->used_page_count += 1 << order;

            return addr;
        }
    }

    return -1;
}

ssize_t alloc_page(void) { return alloc_pages(1); }
void free_page(u64 addr) { free_pages(addr, 1); }
void free_region(u64 addr, size_t count) { free_pages(addr, count); }
