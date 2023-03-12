#pragma once

#include <pci.h>

#define RTL8139_VENDOR_ID 0x10ec
#define RTL8139_DEVICE_ID 0x8139

#define ETHERTYPE_IPV4	0x0800
#define ETHERTYPE_ARP	0x0806
#define ETHERTYPE_IPV6	0x86dd

struct eth_header {
    u8 dst_mac[6];
    u8 src_mac[6];
    u16 eth_type;
} __attribute((packed));

int init_rtl8139(void);
void rtl8139_send(u8 *dst_mac, u16 eth_type, void *payload, u16 size);
void debug_print_mac_addr(void);
