#pragma once

#include <types.h>

#define VERSION_BITS 4
#define IHL_BITS 4
#define DSCP_BITS 6
#define ECN_BITS 2
#define FLAGS_BITS 3
#define FRAGMENT_BITS 13

#define IPV4_VERSION 4
#define IPV6_VERSION 6

#define IP_PROTOCOL_ICMP 1
#define IP_PROTOCOL_TCP 6
#define IP_PROTOCOL_UDP 17

struct net_frame;

struct ipv4_header {
    // 4 bit ihl, 4 bit version
    u8 version_ihl;
    // 6 bit dscp, 2 bit ecn
    u8 dscp_ecn;
    u16 total_len;
    u16 id;
    // 3 bit flags, 13 bit fragment_offset
    u16 flags_fragment;
    u8 ttl;
    u8 protocol;
    u16 checksum;
    u8 src_ip[4];
    u8 dst_ip[4];
} __attribute__((packed));

int ipv4_send_frame(struct net_frame *frame, u8 *ip_addr, u8 protocol);
void ipv4_receive_frame(struct net_frame *frame);
void ipv6_receive_frame(struct net_frame *frame);
