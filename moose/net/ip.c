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

void ipv4_send_frame(u8 *ip_addr, u8 protocol, void *payload, u16 size) {
    u8 dst_mac[6];
    if (arp_get_mac(ip_addr, dst_mac))
        return;

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

    eth_send_frame(dst_mac, ETH_TYPE_IPV4, frame, size);
}

void ipv4_receive_frame(void *frame, u16 size) {
    frame++;
    size++;
}

void ipv6_receive_frame(void *frame, u16 size) {
    frame++;
    size++;
}
