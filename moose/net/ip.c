#include <net/ip.h>
#include <net/arp.h>
#include <net/eth.h>
#include <net/common.h>
#include <endian.h>
#include <mm/kmem.h>

u8 rtl8139_ipaddr[] = {10, 0, 2, 15};
u8 gateway_ipaddr[] = {10, 0, 2, 2};

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

void ipv4_send(u8 *ipaddr, u8 protocol, void *payload, u16 size) {
    u8 dst_mac[6];
    if (arp_get_mac(ipaddr, dst_mac))
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
    memcpy(header->src_ip, rtl8139_ipaddr, sizeof(rtl8139_ipaddr));
    memcpy(header->dst_ip, ipaddr, sizeof(ipaddr));
    header->checksum = 0;
    header->checksum = checksum(header, sizeof(struct ipv4_header));

    memcpy(frame + sizeof(struct ipv4_header), payload, size);

    send_eth_frame(dst_mac, ETH_TYPE_IPV4, frame, size);
}
