#include <net/ip.h>
#include <net/arp.h>
#include <net/eth.h>
#include <net/inet.h>
#include <net/icmp.h>
#include <net/common.h>
#include <endian.h>
#include <assert.h>
#include <mm/kmem.h>

static int is_local_ip_addr(u8 *ip_addr) {
    expects(ip_addr != NULL);

    u8 temp[4];
    memcpy(temp, ip_addr, 4);
    for (u8 i = 0; i < 4; i++)
        temp[i] &= local_net_mask[i];

    return memcmp(temp, local_net_ip_addr, 4) == 0;
}

void ipv4_send_frame(u8 *ip_addr, u8 protocol, void *payload, u16 size) {
    expects(ip_addr != NULL);
    expects(payload != NULL);
    expects(size <= ETH_PAYLOAD_MAX_SIZE);

    u8 dst_mac[6];
    int found;
    if (is_local_ip_addr(ip_addr)) {
        found = (arp_get_mac(ip_addr, dst_mac) == 0);
    } else {
        found = (arp_get_mac(gateway_ip_addr, dst_mac) == 0);
    }
    if (!found) return;

    u8 frame[ETH_PAYLOAD_MAX_SIZE];
    memset(frame, 0, sizeof(struct ipv4_header));

    struct ipv4_header *header = (struct ipv4_header *)frame;
    header->version_ihl |= 4;
    header->version_ihl |= (5 << IHL_BITS);
    header->dscp_ecn = 0;
    header->total_len = htobe16(sizeof(struct ipv4_header) + size);
    header->id = 0;
    header->ttl = 64;
    header->protocol = protocol;
    memcpy(header->src_ip, nic.ip_addr, 4);
    memcpy(header->dst_ip, ip_addr, 4);
    header->checksum = 0;
    header->checksum = checksum(header, sizeof(struct ipv4_header));

    memcpy(header + 1, payload, size);
    u16 frame_size = size + sizeof(struct ipv4_header);

    eth_send_frame(dst_mac, ETH_TYPE_IPV4, frame, frame_size);
}

void ipv4_receive_frame(void *frame) {
    struct ipv4_header *header = (struct ipv4_header *)frame;
    header->total_len = be16toh(header->total_len);
    header->id = be16toh(header->id);
    header->flags_fragment = be16toh(header->flags_fragment);
    u8 version = header->version_ihl & 0xf;
    u8 ihl = header->version_ihl >> IHL_BITS;

    if (version == IPV4_VERSION) {
        void *payload = frame + ihl * 4;
        u16 payload_size = header->total_len - sizeof(*header);

        switch (header->protocol) {
        case IP_PROTOCOL_ICMP:
            icmp_receive_frame(header->src_ip, payload, payload_size);
            break;
        case IP_PROTOCOL_TCP: break;
        case IP_PROTOCOL_UDP: break;
        }
    }
}

void ipv6_receive_frame(__attribute__((unused)) void *frame) {
}
