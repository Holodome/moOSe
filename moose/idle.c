#include <drivers/disk.h>
#include <arch/cpu.h>
#include <idle.h>
#include <shell.h>
#include <device.h>
#include <drivers/pci.h>
#include <fs/fat.h>
#include <kstdio.h>
#include <kthread.h>
#include <net/arp.h>
#include <net/icmp.h>
#include <net/inet.h>
#include <net/ip.h>
#include <net/netdaemon.h>
#include <net/udp.h>
#include <panic.h>
#include <string.h>

__attribute__((noreturn)) void other_task(void) {
    for (;;)
        ;
}

void idle_task(void) {
    init_disk();
    init_shell();

    init_net_daemon();

    print_queue();
    char message[] = "hello world";
    for (int i = 0; i < 5; i++) {
        net_daemon_add_frame(message, strlen(message));
    }
    print_queue();

    launch_task(other_task);

    for (;;)
        wait_for_int();
}
