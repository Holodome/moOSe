#pragma once

#include <types.h>

struct nic {
    u8 mac_addr[6];
    u8 ip_addr[4];
    void (*send_frame)(void *frame, u16 size);
};

extern struct nic nic;

extern u8 gateway_ip_addr[4];
extern u8 dns_ip_addr[4];
extern u8 local_net_mask[4];
extern u8 local_net_ip_addr[4];
extern u8 broadcast_mac_addr[6];

int init_inet(void);
u16 checksum(void *data, size_t size);
void debug_print_frame_hexdump(u8 *frame, size_t size);
void debug_print_mac_addr(u8 *mac_addr);
void debug_print_ip_addr(u8 *ip_addr);
