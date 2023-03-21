#include <net/eth.h>
#include <net/arp.h>
#include <net/inet.h>
#include <net/ip.h>
#include <net/common.h>
#include <mm/kmem.h>
#include <endian.h>
#include <assert.h>
#include <kstdio.h>

void eth_send_frame(u8 *dst_mac_addr, u16 eth_type, void *payload, u16 size) {
    expects(dst_mac_addr != NULL);
    expects(payload != NULL);
    expects(size <= ETH_PAYLOAD_MAX_SIZE);

    u8 frame[ETH_FRAME_MAX_SIZE];
    struct eth_header *header = (struct eth_header*)frame;

    memcpy(header->dst_mac, dst_mac_addr, sizeof(header->dst_mac));
    memcpy(header->src_mac, nic.mac_addr, sizeof(header->src_mac));
    header->eth_type = htobe16(eth_type);

    memcpy(header + 1, payload, size);

    u16 frame_size = sizeof(*header) + size;
    // pad frame with zeros, to be at least mininum size
    if (size < ETH_PAYLOAD_MIN_SIZE) {
        memset(frame + frame_size, 0, ETH_PAYLOAD_MIN_SIZE - size);
        frame_size = sizeof(*header) + ETH_PAYLOAD_MIN_SIZE;
    }

//    debug_print_frame_hexdump(frame, frame_size);

    nic.send_frame(frame, frame_size);
}

void eth_receive_frame(void *frame, u16 size) {
    expects(frame != NULL);
    expects(size <= ETH_FRAME_MAX_SIZE);

//    debug_print_frame_hexdump(frame, size);

    struct eth_header *header = (struct eth_header *)frame;
    header->eth_type = be16toh(header->eth_type);
    void *payload = header + 1;

    switch (header->eth_type) {
    case ETH_TYPE_ARP:
        arp_receive_frame(payload); break;
    case ETH_TYPE_IPV4:
        ipv4_receive_frame(payload); break;
    case ETH_TYPE_IPV6:
        ipv6_receive_frame(payload); break;
    }
}
