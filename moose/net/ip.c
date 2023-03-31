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

static int is_local_ip_addr(const u8 *ip_addr) {
    u8 temp[4];
    memcpy(temp, ip_addr, 4);
    for (u8 i = 0; i < 4; i++)
        temp[i] &= local_net_mask[i];

    return memcmp(temp, local_net_ip_addr, 4) == 0;
}

void ipv4_send_frame(struct net_frame *frame, const u8 *ip_addr, u8 protocol) {
    u8 dst_mac[6];
    const u8 *src_mac = is_local_ip_addr(ip_addr) ? ip_addr : gateway_ip_addr;
    if (arp_get_mac(src_mac, dst_mac)) {
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
    memcpy(header->src_ip, nic.ip_addr, 4);
    memcpy(header->dst_ip, ip_addr, 4);
    header->checksum = 0;
    header->checksum = inet_checksum(header, sizeof(struct ipv4_header));
    header->checksum = htobe16(header->checksum);

    memcpy(&frame->ipv4_header, frame->head, sizeof(*header));
    frame->inet_kind = INET_KIND_IPV4;

    eth_send_frame(frame, dst_mac, ETH_TYPE_IPV4);
}

void ipv4_receive_frame(struct net_frame *frame) {
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
            icmp_receive_frame(frame);
            break;
        case IP_PROTOCOL_TCP:
            break;
        case IP_PROTOCOL_UDP:
            udp_receive_frame(frame);
            break;
        default:
            kprintf("ip protocol %d is unsupported\n", header->protocol);
        }
    }
}

void ipv6_receive_frame(__attribute__((unused)) struct net_frame *frame) {
}
