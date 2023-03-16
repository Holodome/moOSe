#include <net/eth.h>
#include <net/inet.h>
#include <net/common.h>
#include <endian.h>
#include <mm/kmem.h>

void send_eth_frame(u8 *dst_mac_addr, u16 eth_type, void *payload, u16 size) {
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

    nic.send_frame(frame, frame_size);
}
