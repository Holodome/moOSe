#include <arch/amd64/ata.h>
#include <arch/amd64/idt.h>

#include <arch/amd64/keyboard.h>
#include <arch/amd64/memory_map.h>
#include <arch/amd64/rtc.h>
#include <arch/amd64/virtmem.h>
#include <arch/cpu.h>
#include <kernel.h>
#include <kmem.h>
#include <kthread.h>
#include <vmalloc.h>

#include <assert.h>
#include <bitops.h>
#include <disk.h>
#include <errno.h>
#include <fs/fat.h>
#include <kmalloc.h>
#include <kstdio.h>
#include <physmem.h>
#include <shell.h>
#include <tty.h>

static void zero_bss(void) {
    extern volatile u64 *__bss_start;
    extern volatile u64 *__bss_end;
    volatile u64 *p = __bss_start;
    while (p != __bss_end)
        *p++ = 0;
}

__attribute__((noreturn)) void kmain_(void);
__attribute__((noreturn)) void kmain(void) {
    zero_bss();
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
            j++;
        }
    }

    if (init_phys_mem(ranges, usable_region_count)) {
        kprintf("physical memory init error\n");
        halt_cpu();
    }

    if (init_virt_mem(ranges, usable_region_count)) {
        kprintf("virtual memory init error\n");
        halt_cpu();
    }

    init_kinit_thread();
    kmain_();
}

void kmain_(void) {
    disk_init();
    init_rtc();
    init_shell();
    shell_execute_command("HELP");

    for (;;) {
        kprintf("moOSe>");

        char buffer[128];
        ssize_t length = read(tty_device, buffer, sizeof(buffer));
        buffer[length] = 0;
        shell_execute_command(buffer);
    }
}
