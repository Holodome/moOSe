#include <arch/amd64/idt.h>
#include <arch/amd64/memmap.h>
#include <arch/amd64/virtmem.h>
#include <arch/cpu.h>
#include <arch/interrupts.h>
#include <assert.h>
#include <idle.h>
#include <kstdio.h>
#include <mm/kmalloc.h>
#include <mm/kmem.h>
#include <mm/physmem.h>
#include <panic.h>
#include <types.h>
#include <arch/amd64/rtc.h>

static void zero_bss(void) {
    extern u64 __bss_start;
    extern u64 __bss_end;
    // NOTE: Do this instead of memset because we know that address is aligned
    u64 *p = &__bss_start;
    while (p < &__bss_end)
        *p++ = 0;
}

static void init_memory(void) {
    const struct memmap_entry *memmap;
    u32 memmap_size;
    get_memmap(&memmap, &memmap_size);
    int usable_region_count = 0;
    for (u32 i = 0; i < memmap_size; ++i) {
        const struct memmap_entry *entry = memmap + i;
        usable_region_count += entry->type == MULTIBOOT_MEMORY_AVAILABLE;
    }

    struct mem_range *ranges = kmalloc(usable_region_count * sizeof(*ranges));
    expects(ranges);
    for (u32 i = 0, j = 0; i < memmap_size; ++i) {
        const struct memmap_entry *entry = memmap + i;
        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            ranges[j].base = entry->base;
            ranges[j].size = entry->length;
            j++;
        }
    }

    if (init_phys_mem(ranges, usable_region_count))
        panic("failed to initialize physical memory");

    if (init_virt_mem(ranges, usable_region_count))
        panic("failed to initialize virtual memory");
}

__noreturn void kmain(void) {
    zero_bss();
    init_kmalloc();
    init_kstdio();
    kprintf("running moOSe kernel\n");
    kprintf("build %s %s\n", __DATE__, __TIME__);

    init_memory();
    init_interrupts();
    init_idt();
    init_rtc();

    for (;;) {
        wait_for_int();
    }

    /* if (launch_first_task(idle_task)) */
    /*     panic("failed to create idle task"); */

    halt_cpu();
}
