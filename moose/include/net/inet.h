#pragma once

#include <types.h>
#include <net/frame.h>

#define ETH_FRAME_MAX_SIZE 1522
#define ETH_FRAME_MIN_SIZE 64

#define ETH_PAYLOAD_MAX_SIZE 1500
#define ETH_PAYLOAD_MIN_SIZE 46

#define ETH_TYPE_IPV4 0x0800
#define ETH_TYPE_ARP 0x0806
#define ETH_TYPE_IPV6 0x86DD

struct nic {
    u8 mac_addr[6];
    u8 ip_addr[4];
    void (*send_frame)(void *frame, size_t size);
};

extern struct nic nic;

extern u8 gateway_ip_addr[4];
extern u8 dns_ip_addr[4];
extern u8 local_net_mask[4];
extern u8 local_net_ip_addr[4];
extern u8 broadcast_mac_addr[6];

int init_inet(void);
u16 inet_checksum(void *data, size_t size);
void debug_print_frame_hexdump(void *frame, size_t size);
void debug_print_mac_addr(u8 *mac_addr);
void debug_print_ip_addr(u8 *ip_addr);
