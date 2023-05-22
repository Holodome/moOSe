#pragma once

#include <net/device.h>
#include <net/inet.h>
#include <types.h>

struct net_frame;

struct eth_header {
    struct mac_addr dst_mac;
    struct mac_addr src_mac;
    u16 eth_type;
};
static_assert(sizeof(struct eth_header) == 14);

void eth_send_frame(struct net_device *dev, struct net_frame *frame,
                    const struct mac_addr *dst_addr, u16 eth_type);
void eth_receive_frame(struct net_device *dev, struct net_frame *frame);
