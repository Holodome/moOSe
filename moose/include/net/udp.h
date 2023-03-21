#pragma once

#include <types.h>

struct udp_header {
    u16 src_port;
    u16 dst_port;
    u16 len;
    u16 checksum;
};

void udp_send_frame(u8 *dst_ip_addr, u16 src_port, u16 dst_port,
                    void *payload, u16 size);
void udp_receive_frame(u8 *frame);
