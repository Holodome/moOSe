#include <disk.h>
#include <idle.h>
#include <kstdio.h>
#include <kthread.h>
#include <shell.h>
#include <arch/cpu.h>
#include <drivers/pci.h>
#include <net/inet.h>
#include <net/arp.h>
#include <net/ip.h>

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

    u8 mac_addr[6];
    if (arp_get_mac(gateway_ip_addr, mac_addr)) {
        kprintf("can't find mac for this ip address\n");
        halt_cpu();
    }
    debug_print_mac_addr(mac_addr);

    char *message = "Hello world!\n";
    ipv4_send_frame(gateway_ip_addr, 1, message, strlen(message));

    launch_task(other_task);

    for (;;)
        ;
}
