#include <net/ip.h>
#include <net/arp.h>
#include <net/eth.h>
#include <net/inet.h>
#include <net/common.h>
#include <endian.h>
#include <mm/kmem.h>

static u16 checksum(void *data, size_t size) {
    u64 sum = 0;

    while(size > 1)  {
        sum += *(u16 *)data++;
        size -= 2;
    }

    if (size > 0)
        sum += *(u8 *)data;

    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;
}

static int is_local_ip_addr(u8 *ip_addr) {
    u8 temp[4];
    memcpy(temp, ip_addr, 4);
    for (u8 i = 0; i < 4; i++)
        temp[i] &= local_net_mask[i];

    return memcmp(ip_addr, local_net_ip_addr, 4) == 0;
}

void ipv4_send_frame(u8 *ip_addr, u8 protocol, void *payload, u16 size) {
    u8 dst_mac[6];
    int found;
    if (is_local_ip_addr(ip_addr)) {
        found = (arp_get_mac(ip_addr, dst_mac) == 0);
    } else {
        found = (arp_get_mac(gateway_ip_addr, dst_mac) == 0);
    }
    if (!found) return;

    u8 frame[ETH_PAYLOAD_MAX_SIZE];

    struct ipv4_header *header = (struct ipv4_header *)frame;
    header->ihl = 5;
    header->version = 4;
    header->dscp = 0;
    header->ecn = 0;
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

void ipv4_receive_frame(__attribute__((unused)) void *frame,
                        __attribute__((unused)) u16 size) {
}

void ipv6_receive_frame(__attribute__((unused)) void *frame,
                        __attribute__((unused)) u16 size) {
}
