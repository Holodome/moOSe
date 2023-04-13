#pragma once

#include <types.h>

#define ICMP_ECHO_REPLY 0
#define ICMP_ECHO_REQUEST 8

struct net_frame;

struct icmp_header {
    u8 type;
    u8 code;
    u16 checksum;
    u32 rest;
};

static_assert(sizeof(struct icmp_header) == 8);

void icmp_send_echo_request(struct net_frame *frame, const u8 *ip_addr);
void icmp_receive_frame(struct net_frame *frame);
