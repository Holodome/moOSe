#include <assert.h>
#include <drivers/rtl8139.h>
#include <endian.h>
#include <kstdio.h>
#include <net/arp.h>
#include <net/device.h>
#include <net/frame.h>
#include <net/inet.h>
#include <net/netdaemon.h>

u8 gateway_ip_addr[4] = {10, 0, 2, 2};
u8 dns_ip_addr[4] = {10, 0, 2, 3};
u8 local_net_ip_addr[4] = {10, 0, 2, 0};
u8 local_net_mask[4] = {255, 255, 255, 0};
u8 broadcast_mac_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

int init_inet(void) {
    struct net_device *rtl8139 = create_rtl8139();
    expects(rtl8139 != NULL);

    rtl8139->ops->open(rtl8139);

    kprintf("rtl8139 ");
    debug_print_mac_addr(rtl8139->mac_addr);

    expects(init_net_frames() == 0);
    expects(init_arp_cache() == 0);
    expects(init_net_daemon() == 0);

    return 0;
}

u16 inet_checksum(const void *data, size_t size) {
    const u16 *ptr = data;
    u32 sum = 0;

    while (size > 1) {
        sum += htobe16(*ptr++);
        size -= 2;
    }

    if (size > 0)
        sum += htobe16(*(u8 *)data << 16);

    sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;
}

void debug_print_mac_addr(const u8 *mac_addr) {
    kprintf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", mac_addr[0], mac_addr[1],
            mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
}

void debug_print_ip_addr(const u8 *ip_addr) {
    kprintf("IP: %d.%d.%d.%d\n", ip_addr[0], ip_addr[1], ip_addr[2],
            ip_addr[3]);
}

void debug_print_frame_hexdump(const void *frame, size_t size) {
    const u8 *data = frame;
    size_t dump_lines = size / 16;
    if (size % 16 != 0)
        dump_lines++;

    for (size_t i = 0; i < dump_lines; i++) {
        kprintf("%06lx ", i * 16);
        for (u8 j = 0; j < 16; j++)
            kprintf("%02x ", data[i * 16 + j]);
        kprintf("\n");
    }
}
