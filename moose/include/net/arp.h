#pragma once

#include <types.h>

#define ETH_HW_TYPE 1

#define ARP_REQUEST 1
#define ARP_REPLY 2

struct arp_header {
    u16 hw_type;
    u16 protocol_type;
    u8 hw_len;
    u8 protocol_len;
    u16 operation;
    u8 src_mac[6];
    u8 src_ip[4];
    u8 dst_mac[6];
    u8 dst_ip[4];
} __attribute((packed));

int init_arp_cache(void);
void arp_send_request(u8 *ip_addr);
void arp_send_reply(void *frame);
void arp_receive_frame(void *frame);
int arp_get_mac(u8 *ip_addr, u8 *mac_addr);
