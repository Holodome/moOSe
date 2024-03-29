#include <moose/arch/amd64/idt.h>
#include <moose/arch/amd64/memmap.h>
#include <moose/arch/amd64/rtc.h>
#include <moose/arch/amd64/virtmem.h>
#include <moose/arch/cpu.h>
#include <moose/arch/interrupts.h>
#include <moose/assert.h>
#include <moose/kstdio.h>
#include <moose/mm/kmalloc.h>
#include <moose/mm/physmem.h>
#include <moose/param.h>
#include <moose/sched/sched.h>

static void zero_bss(void) {
    extern u64 __bss_start;
    extern u64 __bss_end;
    // NOTE: Do this instead of memset because we know that address is aligned
    u64 *p = &__bss_start;
    while (p < &__bss_end)
        *p++ = 0;
}

static size_t get_total_kernel_size(void) {
    extern char __start;
    extern char __end;
    return &__end - &__start;
}

int init_virt_mem(const struct mem_range *ranges, size_t range_count) {
    // preallocate kernel physical space
    size_t kernel_size_in_pages = get_total_kernel_size() >> PAGE_SIZE_BITS;
    if (alloc_region(KERNEL_PHYSICAL_BASE, kernel_size_in_pages))
        return -1;

    // preallocate currently used page tables
    if (alloc_region(0, 8))
        return -1;

    // all physical memory map to PHYSMEM_VIRTUAL_BASE
    for (size_t i = 0; i < range_count; i++) {
        if (map_virtual_region(
                ranges[i].base, PHYSMEM_VIRTUAL_BASE + ranges[i].base,
                align_po2(ranges[i].size, PAGE_SIZE) >> PAGE_SIZE_BITS))
            return -1;
    }

    // physical memory identity unmap
    unmap_virtual_region(0x0, IDENTITY_MAP_SIZE >> PAGE_SIZE_BITS);

    return 0;
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

__used static void other_task(void *arg __unused) {
    for (;;)
        pause();
}

__noreturn void kmain(void) {
    zero_bss();
    init_kmalloc();
    init_kstdio();
    kprintf("running moOSe kernel\n");
    kprintf("build %s %s\n", __DATE__, __TIME__);

    init_cpu();
    init_memory();

    init_interrupts();
    init_idt();
    init_scheduler();
    init_rtc();

    launch_process("other", other_task, NULL);

    for (;;) {
        kprintf("hello\n");
        wait_for_int();
    }

    halt_cpu();
}
