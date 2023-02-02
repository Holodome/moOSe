#include <arch/amd64/ata.h>
#include <arch/amd64/idt.h>

#include <arch/amd64/keyboard.h>
#include <arch/amd64/memory_map.h>
#include <kmem.h>
#include <arch/amd64/physmem.h>

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

    kprintf(">");
    char buffer[32];
    ssize_t len = tty_read(buffer, sizeof(buffer));
    kprintf("received %.*s (%d)\n", (int)len, buffer, len);

    init_phys_manager();

    void *p;
    if (alloc_page_block(&p, 10240000) < 0)
        kprintf("error: allocate %d blocks\n", 10240000);
    else
        kprintf("size = %d, addr = %#-16x\n", 10240000, (u64) p);

    if (alloc_page_block(&p, 1024) < 0)
        kprintf("error: allocate %d blocks\n", 1024);
    else
        kprintf("size = %d, addr = %#-16x\n", 1024, (u64) p);

    void *blocks[16];
    for (size_t i = 0; i < 8; i++) {
        if (alloc_page_block(&blocks[i], 64) < 0)
            kprintf("alloc error\n");
        kprintf("size = %d, addr = %#-16x\n", 64, (u64) blocks[i]);
    }

    for (size_t i = 0; i < 3; i++)
        free_page_block(blocks[i + 1], 64);

    for (size_t i = 0; i < 4; i++) {
        if (alloc_page_block(&blocks[i], 64) < 0)
            kprintf("alloc error\n");
        kprintf("size = %d, addr = %#-16x\n", 64, (u64) blocks[i]);
    }

    for (size_t i = 0; i < 2; i++) {
        if (alloc_page_block(&blocks[i], 1) < 0)
            kprintf("alloc error\n");
        kprintf("size =  %d, addr = %#-16x\n", 1, (u64) blocks[i]);
    }

    for (;;)
        ;
}
