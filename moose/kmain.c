#include <arch/amd64/ata.h>
#include <arch/amd64/idt.h>

#include <arch/amd64/keyboard.h>
#include <arch/amd64/memory_map.h>
#include <arch/amd64/physmem.h>
#include <arch/amd64/virtmem.h>
#include <kmem.h>

#include <kstdio.h>
#include <tty.h>
#include <errno.h>
#include <disk.h>
#include <fs/fat.h>

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
    if (init_phys_mem(memmap, memmap_size))
        kprintf("physical memory init error");

    if (init_virt_mem(memmap, memmap_size))
        kprintf("virtual memory init error");

    for (;;)
        ;
}
