#include <net/inet.h>
#include <kstdio.h>
#include <drivers/rtl8139.h>
#include <mm/kmem.h>
#include <endian.h>
#include <net/common.h>
#include <net/eth.h>

struct nic nic;
static u8 nic_ip_addr[4] = {10, 0, 2, 15};
u8 gateway_ip_addr[4] = {10, 0, 2, 2};
u8 broadcast_mac_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

int init_inet(void) {
    if (init_rtl8139(nic.mac_addr))
        return -1;

    memcpy(nic.ip_addr, nic_ip_addr, 4);
    debug_print_mac_addr(nic.mac_addr);

    nic.send_frame = rtl8139_send;

    return 0;
}

void handle_frame(void *frame, u16 size) {
    struct eth_header *header = (struct eth_header *)frame;
    header->eth_type = be16toh(header->eth_type);

    if (header->eth_type == ETH_TYPE_ARP) {

    } else if (header->eth_type == ETH_TYPE_IPV4) {

    }
}

void debug_print_mac_addr(u8 *mac_addr) {
    kprintf("MAC: %01x:%01x:%01x:%01x:%01x:%01x\n",
            mac_addr[0], mac_addr[1],
            mac_addr[2], mac_addr[3],
            mac_addr[4], mac_addr[5]);
}

void debug_print_frame_hexdump(u8 *frame, size_t size) {
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
