#include <arch/amd64/ata.h>
#include <arch/amd64/idt.h>

#include <arch/amd64/keyboard.h>
#include <arch/amd64/memory_map.h>
#include <arch/amd64/rtc.h>
#include <arch/amd64/virtmem.h>
#include <arch/processor.h>
#include <kernel.h>
#include <kmem.h>

#include <bitops.h>
#include <disk.h>
#include <errno.h>
#include <fs/fat.h>
#include <kmalloc.h>
#include <kstdio.h>
#include <physmem.h>
#include <tty.h>

static void zero_bss(void) {
    extern volatile u64 *__bss_start;
    extern volatile u64 *__bss_end;
    volatile u64 *p = __bss_start;
    while (p != __bss_end)
        *p++ = 0;
}

static void fixup_gdt(void) {
    struct {
        u16 size;
        u64 offset;
    } __attribute__((packed)) gdtr;
    asm volatile("sgdt %0" : "=m"(gdtr));
    gdtr.offset = FIXUP_ADDR(gdtr.offset);
    asm volatile("lgdt %0" : : "m"(gdtr));
}

__attribute__((noreturn)) void kmain(void) {
    zero_bss();
    fixup_gdt();
    init_memory();
    kputs("running moOSe kernel");
    kprintf("build %s %s\n", __DATE__, __TIME__);

    const struct memmap_entry *memmap;
    u32 memmap_size;
    get_memmap(&memmap, &memmap_size);
    kprintf("Bios-provided physical RAM map:\n");
    int usable_region_count = 0;
    for (u32 i = 0; i < memmap_size; ++i) {
        const struct memmap_entry *entry = memmap + i;
        kprintf("%#016llx-%#016llx: %s(%u)\n", (unsigned long long)entry->base,
                (unsigned long long)(entry->base + entry->length),
                get_memmap_type_str(entry->type), (unsigned)entry->type);
        usable_region_count += entry->type == MULTIBOOT_MEMORY_AVAILABLE;
    }

    setup_idt();
    init_keyboard();
    struct mem_range *ranges = kmalloc(usable_region_count * sizeof(*ranges));
    for (u32 i = 0, j = 0; i < memmap_size; ++i) {
        const struct memmap_entry *entry = memmap + i;
        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            ranges[j].base = entry->base;
            ranges[j].size = entry->length;
            ++j;
        }
    }

    if (init_phys_mem(ranges, usable_region_count)) {
        kprintf("physical memory init error\n");
        goto halt;
    }

    if (init_virt_mem(memmap, memmap_size)) {
        kprintf("virtual memory init error\n");
        goto halt;
    }

    disk_init();
    init_rtc();

    struct pfatfs fs = {.device = disk_part_dev};
    int result = pfatfs_mount(&fs);
    if (result == 0) {

        struct pfatfs_file file = {0};
        int result = pfatfs_open(&fs, "kernel1.bin", &file);
        if (result == 0) {
            kprintf("opened file %11s\n", file.name);
        }
    }

    u32 secs = 0;
    for (;;) {
        u32 new_secs = get_seconds();
        if (new_secs != secs) {
            /* kprintf("time %u\n", new_secs); */
            secs = new_secs;
        }
    }

halt:
    halt();
    for (;;)
        ;
}
