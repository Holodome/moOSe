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

__attribute__((noreturn)) void idle_task(void);

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

    if (init_kinit_thread(idle_task)) {
        kprintf("failed to init idle task\n");
        irq_disable();
        halt_cpu();
    }

    __builtin_unreachable();
}

static volatile struct task *old_task;
void do_stuff(void) {
    kprintf("do stuff\n");
    kprintf("current %p %p\n", current->stack, current->eip);
    context_switch(current, old_task);
    kprintf("current %p %p\n", current->stack, current->eip);
    for (;;)
        ;
}

void idle_task(void) {
    kprintf("current %p %p\n", current->stack, current->eip);
    disk_init();
    init_rtc();
    init_shell();

    old_task = current;
    struct task *new_task = create_task(do_stuff);
    context_switch(current, new_task);
    kprintf("initialized\n");

    for (;;) {
        halt_cpu();
    }
}
