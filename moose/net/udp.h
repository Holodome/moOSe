#pragma once

#include <moose/types.h>

struct net_frame;

struct udp_header {
    u16 src_port;
    u16 dst_port;
    u16 len;
    u16 checksum;
};

static_assert(sizeof(struct udp_header) == 8);

void udp_send_frame(struct net_frame *frame, const u8 *dst_ip_addr,
                    u16 src_port, u16 dst_port);
void udp_receive_frame(struct net_frame *frame);
