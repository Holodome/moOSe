#include <disk.h>
#include <idle.h>
#include <kstdio.h>
#include <kthread.h>
#include <shell.h>
#include <arch/cpu.h>
#include <drivers/pci.h>
#include <net/inet.h>
#include <net/icmp.h>
#include <net/arp.h>
#include <net/ip.h>
#include <net/netdaemon.h>

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
    kprintf("gateway ");
    debug_print_mac_addr(mac_addr);

    if (arp_get_mac(dns_ip_addr, mac_addr)) {
        kprintf("can't find mac for this ip address\n");
        halt_cpu();
    }
    kprintf("dns ");
    debug_print_mac_addr(mac_addr);

    icmp_send_echo_request(dns_ip_addr);

    launch_task(other_task);

    for (;;)
        ;
}
