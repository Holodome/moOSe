#pragma once

#include <types.h>

struct udp_header {
    u16 src_port;
    u16 dst_port;
    u16 len;
    u16 checksum;
} __attribute__((packed));

int udp_send_frame(u8 *dst_ip_addr, u16 src_port, u16 dst_port, void *payload,
                   size_t size);
void udp_receive_frame(void *frame);
