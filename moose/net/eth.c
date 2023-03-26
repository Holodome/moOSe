#include <net/eth.h>
#include <net/arp.h>
#include <net/frame.h>
#include <net/inet.h>
#include <net/ip.h>
#include <string.h>
#include <endian.h>
#include <assert.h>
#include <kstdio.h>
#include <errno.h>

int eth_send_frame(u8 *dst_mac_addr, u16 eth_type, void *payload, size_t size) {
    expects(size <= ETH_PAYLOAD_MAX_SIZE);

    struct net_frame *frame = alloc_net_frame();
    if (frame == NULL)
        return -ENOMEM;

    struct eth_header *header = frame->data;

    memcpy(header->dst_mac, dst_mac_addr, sizeof(header->dst_mac));
    memcpy(header->src_mac, nic.mac_addr, sizeof(header->src_mac));
    header->eth_type = htobe16(eth_type);

    memcpy(header + 1, payload, size);

    size_t frame_size = sizeof(*header) + size;
    // pad frame with zeros, to be at least mininum size
    if (size < ETH_PAYLOAD_MIN_SIZE) {
        memset(frame->data + frame_size, 0, ETH_PAYLOAD_MIN_SIZE - size);
        frame_size = sizeof(*header) + ETH_PAYLOAD_MIN_SIZE;
    }
    frame->size = frame_size;

#if 0
    debug_print_frame_hexdump(frame, size);
#endif

    nic.send_frame(frame->data, frame->size);
    free_net_frame(frame);

    return 0;
}

void eth_receive_frame(void *frame, size_t size) {
    expects(size <= ETH_FRAME_MAX_SIZE);

#if 0
    debug_print_frame_hexdump(frame, size);
#endif

    struct eth_header *header = frame;
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
