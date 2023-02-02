#include <arch/amd64/ata.h>
#include <arch/amd64/idt.h>

#include <arch/amd64/keyboard.h>
#include <arch/amd64/memory_map.h>
#include <arch/amd64/physmem.h>
#include <kmem.h>

#include <disk.h>
#include <errno.h>
#include <fs/fat.h>
#include <kstdio.h>
#include <tty.h>

__attribute__((noreturn)) void kmain(void) {
    kputs("running moOSe kernel");
    kprintf("build %s %s\n", __DATE__, __TIME__);

    const struct memmap_entry *memmap;
    u32 memmap_size;
    get_memmap(&memmap, &memmap_size);
    kprintf("Bios-provided physical RAM map:\n");
    for (u32 i = 0; i < memmap_size; ++i) {
        const struct memmap_entry *entry = memmap + i;
        kprintf("%#016llx-%#016llx: %s(%u)\n", (unsigned long long)entry->base,
                (unsigned long long)(entry->base + entry->length),
                get_memmap_type_str(entry->type), (unsigned)entry->type);
    }

    setup_idt();
    init_keyboard();
    init_phys_manager();
    disk_init();


    struct pfatfs fs = {0};
    int result = pfatfs_mount(&fs);
    if (result == 0) {

        struct pfatfs_file file = {0};
        int result = pfatfs_open(&fs, "kernel1.bin", &file);
        if (result == 0) {
            kprintf("opened file %11s\n", file.name);
        }
    }

    for (;;)
        ;
}
