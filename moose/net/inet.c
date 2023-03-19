#include <net/inet.h>
#include <kstdio.h>
#include <assert.h>
#include <drivers/rtl8139.h>
#include <mm/kmem.h>
#include <net/netdaemon.h>
#include <net/arp.h>

struct nic nic;

static u8 nic_ip_addr[4] =  {10, 0, 2, 15};
u8 gateway_ip_addr[4] =     {10, 0, 2, 2};
u8 local_net_ip_addr[4] =   {10, 0, 2, 0};
u8 local_net_mask[4] =      {255, 255, 255, 0};
u8 broadcast_mac_addr[6] =  {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

int init_inet(void) {
    if (init_rtl8139(nic.mac_addr))
        return -1;

    memcpy(nic.ip_addr, nic_ip_addr, 4);
    debug_print_mac_addr(nic.mac_addr);

    nic.send_frame = rtl8139_send;

    if (init_arp_cache())
        return -1;

    if (init_net_daemon())
        return -1;

    return 0;
}

void debug_print_mac_addr(u8 *mac_addr) {
    expects(mac_addr != NULL);
    kprintf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
            mac_addr[0], mac_addr[1],
            mac_addr[2], mac_addr[3],
            mac_addr[4], mac_addr[5]);
}

void debug_print_frame_hexdump(u8 *frame, size_t size) {
    expects(frame != NULL);
    size_t dump_lines = size / 16;
    if (size % 16 != 0)
        dump_lines++;

    for (size_t i = 0; i < dump_lines; i++) {
        kprintf("%06lx ", i * 16);
        for (u8 j = 0; j < 16; j++)
            kprintf("%02x ", frame[i * 16 + j]);
        kprintf("\n");
    }
}
