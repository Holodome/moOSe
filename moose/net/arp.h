#pragma once

#include <net/device.h>
#include <types.h>

#define ETH_HW_TYPE 1

#define ARP_REQUEST 1
#define ARP_REPLY 2

struct net_frame;

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
};
static_assert(sizeof(struct arp_header) == 28);

int init_arp_cache(void);
void destroy_arp_cache(void);

void arp_send_request(struct net_device *dev, struct net_frame *frame,
                      const struct ip_addr *ip_addr);
void arp_receive_frame(struct net_device *dev, struct net_frame *frame);
int arp_get_mac(struct net_device *dev, const struct ip_addr *ip_addr, struct mac_addr *mac_addr);
void debug_clear_arp_cache(void);
