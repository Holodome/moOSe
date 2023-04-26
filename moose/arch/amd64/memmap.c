#include <moose/arch/amd64/memmap.h>
#include <moose/param.h>

#define MEMMAP_ADDR FIXUP_PTR((u8 *)0x500)

void get_memmap(const struct memmap_entry **map, u32 *count) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
    *count = *(u32 *)MEMMAP_ADDR;
    *map = (void *)((char *)MEMMAP_ADDR + 4);
#pragma GCC diagnostic pop
}

const char *get_memmap_type_str(u32 type) {
    static const char *strings[] = {"usable", "reserved", "acpi data",
                                    "acpi nvs", "bad"};
    --type;
    const char *result = NULL;
    if (type < ARRAY_SIZE(strings)) {
        result = strings[type];
    }

    return result;
}
