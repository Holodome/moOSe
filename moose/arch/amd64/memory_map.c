#include "memory_map.h"

#define MEMMAP_ADDR ((void *)0x500)

void get_memmap(const struct memmap_entry **map, u32 *count) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
    *count = *(u32 *)MEMMAP_ADDR;
    *map = (void *)((char *)MEMMAP_ADDR + 4);
#pragma GCC diagnostic pop
}
