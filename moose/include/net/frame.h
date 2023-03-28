#pragma once

#include <types.h>
#include <list.h>
#include <net/inet.h>
#include <net/eth.h>
#include <net/arp.h>
#include <net/icmp.h>
#include <net/ip.h>
#include <net/udp.h>

#define MAX_HEADER_SIZE 256
#define FRAME_BUFFER_SIZE (ETH_PAYLOAD_MAX_SIZE + MAX_HEADER_SIZE)

enum link_type {
    LINK_TYPE_ETH
};

enum inet_type {
    INET_TYPE_IPV4,
    INET_TYPE_ARP
};

enum transport_type {
    TRANSPORT_TYPE_UDP,
    TRANSPORT_TYPE_ICMP
};

enum frame_type {
    SEND_FRAME,
    RECEIVE_FRAME
};

struct net_frame {
    enum frame_type type;

    enum link_type link_type;
    union {
        struct eth_header eth_header;
    };

    enum inet_type inet_type;
    union {
        struct ipv4_header ipv4_header;
        struct arp_header arp_header;
    };

    enum transport_type transport_type;
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
struct net_frame *get_free_net_frame(enum frame_type type);
void release_net_frame(struct net_frame *frame);

void push_net_frame_head(struct net_frame *frame, size_t offset);
void pull_net_frame_head(struct net_frame *frame, size_t offset);
void inc_net_frame_size(struct net_frame *frame, size_t increment);
size_t get_net_frame_size(struct net_frame *frame);
