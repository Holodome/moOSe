#include <arch/amd64/idt.h>
#include <arch/amd64/keyboard.h>
#include <arch/amd64/memmap.h>
#include <arch/amd64/rtc.h>
#include <arch/amd64/virtmem.h>
#include <arch/cpu.h>
#include <idle.h>
#include <kstdio.h>
#include <kthread.h>
#include <mm/kmalloc.h>
#include <mm/kmem.h>
#include <mm/physmem.h>
#include <rtl8139.h>
#include <types.h>
#include <pci.h>

static void zero_bss(void) {
    extern volatile u64 *__bss_start;
    extern volatile u64 *__bss_end;
    volatile u64 *p = __bss_start;
    while (p != __bss_end)
        *p++ = 0;
}

__attribute__((noreturn)) void kmain(void) {
    zero_bss();
    init_kmalloc();
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

    init_idt();
    init_keyboard();
    struct mem_range *ranges = kmalloc(usable_region_count * sizeof(*ranges));
    for (u32 i = 0, j = 0; i < memmap_size; ++i) {
        const struct memmap_entry *entry = memmap + i;
        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            ranges[j].base = entry->base;
            ranges[j].size = entry->length;
            j++;
        }
    }

    if (init_phys_mem(ranges, usable_region_count)) {
        kprintf("failed to initialize physical memory\n");
        halt_cpu();
    }

    if (init_virt_mem(ranges, usable_region_count)) {
        kprintf("failed to initialize virtual memory\n");
        halt_cpu();
    }

    init_pci();
    struct pci_bus *bus = get_root_bus();
    debug_print_bus(bus);

    if (init_rtl8139()) {
        kprintf("failed to initialize rtl8139\n");
        halt_cpu();
    }
    debug_print_mac_addr();

    rtl8139_send("hello", 6);
    rtl8139_send("world", 6);
    kprintf("messages are sent successfully\n");

    init_rtc();

    if (launch_first_task(idle_task)) {
        kprintf("failed to create idle task\n");
        halt_cpu();
    }

    for (;;)
        halt_cpu();
}

