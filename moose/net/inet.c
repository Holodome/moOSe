#include <drivers/rtl8139.h>
#include <endian.h>
#include <kstdio.h>
#include <net/arp.h>
#include <net/frame.h>
#include <net/inet.h>
#include <net/netdaemon.h>
#include <string.h>

struct nic nic;

static u8 nic_ip_addr[4] = {10, 0, 2, 15};
u8 gateway_ip_addr[4] = {10, 0, 2, 2};
u8 dns_ip_addr[4] = {10, 0, 2, 3};
u8 local_net_ip_addr[4] = {10, 0, 2, 0};
u8 local_net_mask[4] = {255, 255, 255, 0};
u8 broadcast_mac_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

int init_inet(void) {
    int err;
    if ((err = init_net_frames()))
        return err;

    if ((err = init_rtl8139(nic.mac_addr))) {
        destroy_net_frames();
        return err;
    }

    memcpy(nic.ip_addr, nic_ip_addr, 4);
    debug_print_mac_addr(nic.mac_addr);

    nic.send_frame = rtl8139_send;

    if ((err = init_arp_cache())) {
        destroy_rtl8139();
        destroy_net_frames();
        return err;
    }

    if ((err = init_net_daemon())) {
        destroy_arp_cache();
        destroy_rtl8139();
        destroy_net_frames();
        return err;
    }

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
