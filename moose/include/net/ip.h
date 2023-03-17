#pragma once

#include <types.h>

struct ipv4_header {
    u8 ihl : 4;
    u8 version : 4;
    u8 dscp : 6;
    u8 ecn : 2;
    u16 total_len;
    u16 id;
    u16 flags : 3;
    u16 fragment_offset : 13;
    u8 ttl;
    u8 protocol;
    u16 checksum;
    u8 src_ip[4];
    u8 dst_ip[4];
} __attribute__((packed));

extern u8 rtl8139_ipaddr[];
extern u8 gateway_ipaddr[];

void ipv4_send(u8 *ipaddr, u8 protocol, void *payload, u16 size);
void ipv4_receive_frame(void *frame, u16 size);
void ipv6_receive_frame(void *frame, u16 size);
