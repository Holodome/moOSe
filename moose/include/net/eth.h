#pragma once

#include <types.h>
#include <net/interface.h>

struct net_frame;

struct eth_header {
    u8 dst_mac[6];
    u8 src_mac[6];
    u16 eth_type;
} __attribute((packed));

void eth_send_frame(struct net_interface *net_if, struct net_frame *frame,
                    const u8 *dst_mac_addr, u16 eth_type);
void eth_receive_frame(struct net_interface *net_if, struct net_frame *frame);
