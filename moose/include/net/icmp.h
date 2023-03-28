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
} __attribute__((packed));

int icmp_send_echo_request(struct net_frame *frame, u8 *ip_addr);
void icmp_receive_frame(struct net_frame *frame);
