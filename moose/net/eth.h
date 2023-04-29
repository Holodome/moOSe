#pragma once

#include <net/device.h>
#include <types.h>

struct net_frame;

struct eth_header {
    u8 dst_mac[6];
    u8 src_mac[6];
    u16 eth_type;
};
static_assert(sizeof(struct eth_header) == 14);

void eth_send_frame(struct net_device *dev, struct net_frame *frame,
                    const u8 *dst_mac_addr, u16 eth_type);
void eth_receive_frame(struct net_device *dev, struct net_frame *frame);
