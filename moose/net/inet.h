#pragma once

#include <types.h>

#define ETH_FRAME_MAX_SIZE 1522
#define ETH_FRAME_MIN_SIZE 64

#define ETH_PAYLOAD_MAX_SIZE 1500
#define ETH_PAYLOAD_MIN_SIZE 46

#define ETH_TYPE_IPV4 0x0800
#define ETH_TYPE_ARP 0x0806
#define ETH_TYPE_IPV6 0x86DD

#define IPV4_ADDR_LEN 16
#define MAC_ADDR_LEN 18

struct net_frame;

struct ip_addr {
    union {
        u32 bits;
        u8 octets[4];
    } addr;
};

struct mac_addr {
    u8 octets[6];
};

extern struct ip_addr gateway_ip_addr;
extern struct ip_addr dns_ip_addr;
extern struct ip_addr local_net_mask;
extern struct ip_addr local_net_ip_addr;
extern struct mac_addr broadcast_mac_addr;
extern struct mac_addr null_mac_addr;

int ip_addr_equals(const struct ip_addr *a, const struct ip_addr *b);
int mac_addr_equals(const struct mac_addr *a, const struct mac_addr *b);
struct ip_addr *copy_ip_addr(struct ip_addr *dst, const struct ip_addr *src);
struct mac_addr *copy_mac_addr(struct mac_addr *dst,
                               const struct mac_addr *src);
int inet_pton(struct ip_addr *addr, const char *str);
int inet_ntop(char *str, const struct ip_addr *addr);
int is_subnet_ip_addr(const struct ip_addr *ip_addr);

int init_inet(void);
u16 inet_checksum(const void *data, size_t size);
void debug_print_frame_hexdump(const void *frame, size_t size);
void debug_print_mac_addr(const struct mac_addr *mac_addr);
void debug_print_ip_addr(const struct ip_addr *ip_addr);

void mac_addr_sprintf(char *str, const struct mac_addr *mac_addr);
void ip_addr_sprintf(char *str, const struct ip_addr *ip_addr);
