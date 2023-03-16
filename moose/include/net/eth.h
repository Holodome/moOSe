#pragma once

#include <types.h>

struct eth_header {
    u8 dst_mac[6];
    u8 src_mac[6];
    u16 eth_type;
} __attribute((packed));

void send_eth_frame(u8 *dst_mac_addr, u16 eth_type, void *payload, u16 size);
