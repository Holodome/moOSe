#pragma once

#include <list.h>
#include <net/arp.h>
#include <net/eth.h>
#include <net/icmp.h>
#include <net/inet.h>
#include <net/ip.h>
#include <net/udp.h>
#include <types.h>

#define MAX_HEADER_SIZE 256
#define FRAME_BUFFER_SIZE (ETH_PAYLOAD_MAX_SIZE + MAX_HEADER_SIZE)

enum link_kind {
    LINK_KIND_ETH
};

enum inet_kind {
    INET_KIND_IPV4,
    INET_KIND_ARP
};

enum transport_kind {
    TRANSPORT_KIND_UDP,
    TRANSPORT_KIND_ICMP
};

struct net_frame {
    enum link_kind link_kind;
    union {
        struct eth_header eth_header;
    };

    enum inet_kind inet_kind;
    union {
        struct ipv4_header ipv4_header;
        struct arp_header arp_header;
    };

    enum transport_kind transport_kind;
    union {
        struct udp_header udp_header;
        struct icmp_header icmp_header;
    };

    size_t size;

    void *head;
    void *buffer;

    void *payload;
    size_t payload_size;

    struct list_head list;
};

int init_net_frames(void);
struct net_frame *get_empty_send_net_frame(void);
struct net_frame *get_empty_receive_net_frame(void);
void release_net_frame(struct net_frame *frame);

void push_net_frame_head(struct net_frame *frame, size_t offset);
void pull_net_frame_head(struct net_frame *frame, size_t offset);
void inc_net_frame_size(struct net_frame *frame, size_t increment);
