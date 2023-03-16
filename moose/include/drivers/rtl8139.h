#pragma once

#include <types.h>

#define RTL8139_VENDOR_ID 0x10ec
#define RTL8139_DEVICE_ID 0x8139

struct eth_header {
    u8 dst_mac[6];
    u8 src_mac[6];
    u16 eth_type;
} __attribute((packed));

int init_rtl8139(u8 *mac_addr);
void rtl8139_send(u8 *dst_mac, u16 eth_type, void *payload, u16 size);
