#pragma once

#include <types.h>

struct memmap_entry {
    u64 base;
    u64 length;
    u32 type;
    u32 acpi;
} __attribute__((packed));

void get_memmap(const struct memmap_entry **map, u32 *count);
const char *get_memmap_type_str(u32 type);
