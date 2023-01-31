#include <arch/amd64/memory_map.h>
#include <arch/amd64/idt.h>
#include <arch/amd64/ata.h>
#include <kstdio.h>

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

    char buffer[512] = { 0 };
    int a = ata_pio_read(buffer, 2, 1);
    if (a != 0) {
        kprintf("failed to read disk\n");
    } else {
        for (int i = 0; i < 512; ++i) {
            kprintf("%c", buffer[i]);
        }
        kprintf("\n");
    }

    for (;;)
        ;
}
