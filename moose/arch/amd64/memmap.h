#pragma once

#include <types.h>

#define MULTIBOOT_MEMORY_AVAILABLE 1
#define MULTIBOOT_MEMORY_RESERVED 2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS 4
#define MULTIBOOT_MEMORY_BADRAM 5

struct memmap_entry {
    u64 base;
    u64 length;
    u32 type;
    u32 acpi;
} __packed;

void get_memmap(const struct memmap_entry **map, u32 *count);
const char *get_memmap_type_str(u32 type);
