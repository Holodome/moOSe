#pragma once

#include <types.h>

#define ICMP_ECHO_REPLY 0
#define ICMP_ECHO_REQUEST 8

struct icmp_header {
    u8 type;
    u8 code;
    u16 checksum;
    u32 rest;
};

void icmp_send_echo_request(u8 *ip_addr);
void icmp_receive_frame(u8 *ip_addr, void *frame, u16 size);
