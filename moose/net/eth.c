#include <endian.h>
#include <kstdio.h>
#include <net/arp.h>
#include <net/eth.h>
#include <net/frame.h>
#include <net/interface.h>
#include <net/inet.h>
#include <net/ip.h>
#include <string.h>

void eth_send_frame(struct net_interface *net_if, struct net_frame *frame,
                    const u8 *dst_mac_addr, u16 eth_type) {
    pull_net_frame_head(frame, sizeof(struct eth_header));
    struct eth_header *header = frame->head;

    memcpy(header->dst_mac, dst_mac_addr, sizeof(header->dst_mac));
    memcpy(header->src_mac, net_if->mac_addr, sizeof(header->src_mac));
    header->eth_type = htobe16(eth_type);

    // pad frame with zeros, to be at least mininum size
    if (frame->size < ETH_FRAME_MIN_SIZE) {
        memset(frame->head + frame->size, 0, ETH_FRAME_MIN_SIZE - frame->size);
        frame->size = ETH_FRAME_MIN_SIZE;
    }

#if 0
    debug_print_frame_hexdump(frame->head, frame->size);
#endif

    memcpy(&frame->eth_header, frame->head, sizeof(*header));
    frame->link_kind = LINK_KIND_ETH;
    net_if->dev->send(net_if->dev, frame->head, frame->size);
}

void eth_receive_frame(struct net_interface *net_if, struct net_frame *frame) {
    if (frame->size > ETH_FRAME_MAX_SIZE) {
        kprintf("eth receive error: invalid frame size\n");
        return;
    }

#if 0
    debug_print_frame_hexdump(frame->head, frame->size);
#endif

    frame->link_kind = LINK_KIND_ETH;

    struct eth_header *header = frame->head;
    memcpy(&frame->eth_header, frame->head, sizeof(*header));
    push_net_frame_head(frame, sizeof(*header));

    header->eth_type = be16toh(header->eth_type);
    switch (header->eth_type) {
    case ETH_TYPE_ARP:
        arp_receive_frame(frame);
        break;
    case ETH_TYPE_IPV4:
        ipv4_receive_frame(frame);
        break;
    case ETH_TYPE_IPV6:
        ipv6_receive_frame(frame);
        break;
    }
}
