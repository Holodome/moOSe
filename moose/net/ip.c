#include <endian.h>
#include <kstdio.h>
#include <net/arp.h>
#include <net/eth.h>
#include <net/frame.h>
#include <net/icmp.h>
#include <net/inet.h>
#include <net/ip.h>
#include <net/udp.h>
#include <string.h>

#define VERSION_BITS 4
#define IHL_BITS 4
#define DSCP_BITS 6
#define ECN_BITS 2
#define FLAGS_BITS 3
#define FRAGMENT_BITS 13

void ipv4_send_frame(struct net_device *dev, struct net_frame *frame,
                     const struct ip_addr *ip_addr, u8 protocol) {
    struct mac_addr dst_mac;
    struct ip_addr src_ip;
    if (is_subnet_ip_addr(ip_addr)) {
        copy_ip_addr(&src_ip, ip_addr);
    } else {
        copy_ip_addr(&src_ip, &gateway_ip_addr);
    }

    if (arp_get_mac(dev, &src_ip, &dst_mac)) {
        kprintf("failed to get mac addr for ip ");
        debug_print_ip_addr(ip_addr);
        return;
    }

    pull_net_frame_head(frame, sizeof(struct ipv4_header));
    struct ipv4_header *header = frame->head;

    // ihl
    header->version_ihl |= 5;
    // version
    header->version_ihl |= (4 << IHL_BITS);
    header->dscp_ecn = 0;
    header->total_len = htobe16(frame->size);
    header->id = 0;
    header->ttl = 64;
    header->protocol = protocol;
    memcpy(header->src_ip, &dev->ip_addr, 4);
    memcpy(header->dst_ip, ip_addr, 4);
    header->checksum = 0;
    header->checksum = inet_checksum(header, sizeof(struct ipv4_header));
    header->checksum = htobe16(header->checksum);

    memcpy(&frame->ipv4_header, frame->head, sizeof(*header));
    frame->inet_kind = INET_KIND_IPV4;

    eth_send_frame(dev, frame, &dst_mac, ETH_TYPE_IPV4);
}

void ipv4_receive_frame(struct net_device *dev, struct net_frame *frame) {
    struct ipv4_header *header = frame->head;

    header->total_len = be16toh(header->total_len);
    header->id = be16toh(header->id);
    header->flags_fragment = be16toh(header->flags_fragment);
    u8 ihl = header->version_ihl & 0xf;
    u8 version = header->version_ihl >> IHL_BITS;

    memcpy(&frame->ipv4_header, frame->head, sizeof(*header));
    frame->inet_kind = INET_KIND_IPV4;
    push_net_frame_head(frame, ihl * 4);

    if (version == IPV4_VERSION) {
        switch (header->protocol) {
        case IP_PROTOCOL_ICMP:
            icmp_receive_frame(dev, frame);
            break;
        case IP_PROTOCOL_TCP:
            break;
        case IP_PROTOCOL_UDP:
            udp_receive_frame(dev, frame);
            break;
        default:
            kprintf("ip protocol %d is unsupported\n", header->protocol);
        }
    }
}

void ipv6_receive_frame(__unused struct net_device *dev,
                        __unused struct net_frame *frame) {
}
