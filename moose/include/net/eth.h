#pragma once

#include <types.h>

struct eth_header {
    u8 dst_mac[6];
    u8 src_mac[6];
    u16 eth_type;
} __attribute((packed));

void eth_send_frame(u8 *dst_mac_addr, u16 eth_type, void *payload, u16 size);
void eth_receive_frame(void *frame, u16 size);
