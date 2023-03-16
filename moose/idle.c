#include <disk.h>
#include <idle.h>
#include <kstdio.h>
#include <kthread.h>
#include <shell.h>
#include <arch/cpu.h>
#include <drivers/pci.h>
#include <net/inet.h>

__attribute__((noreturn)) void other_task(void) {
    for (;;)
        ;
}

void idle_task(void) {
    init_disk();
    init_shell();

    if (init_pci()) {
        kprintf("failed to initialize pci bus\n");
        halt_cpu();
    }
    struct pci_bus *bus = get_root_bus();
    debug_print_bus(bus);

    if (init_inet()) {
        kprintf("failed to initialize inet system\n");
        halt_cpu();
    }

    launch_task(other_task);

    for (;;)
        ;
}
