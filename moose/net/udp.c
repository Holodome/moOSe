#include <endian.h>
#include <kstdio.h>
#include <net/frame.h>
#include <net/inet.h>
#include <net/ip.h>
#include <net/udp.h>
#include <string.h>

void udp_send_frame(struct net_device *dev, struct net_frame *frame,
                    const u8 *dst_ip_addr, u16 src_port, u16 dst_port) {
    pull_net_frame_head(frame, sizeof(struct udp_header));
    struct udp_header *header = frame->head;

    header->src_port = htobe16(src_port);
    header->dst_port = htobe16(dst_port);
    header->len = frame->size;
    header->checksum = 0;
    header->checksum = htobe16(inet_checksum(header, header->len));

    memcpy(&frame->udp_header, frame->head, sizeof(*header));
    frame->transport_kind = TRANSPORT_KIND_UDP;

    ipv4_send_frame(dev, frame, dst_ip_addr, IP_PROTOCOL_UDP);
}

void udp_receive_frame(__unused struct net_device *dev,
                       struct net_frame *frame) {
    struct udp_header *header = frame->head;

    header->src_port = be16toh(header->src_port);
    header->dst_port = be16toh(header->dst_port);
    header->len = be16toh(header->len);

    push_net_frame_head(frame, sizeof(*header));
    frame->payload = frame->head;
    frame->payload_size = frame->size;

    memcpy(&frame->udp_header, header, sizeof(*header));
    frame->transport_kind = TRANSPORT_KIND_UDP;

    kprintf("UDP payload:\n");
    debug_print_frame_hexdump(frame->payload, frame->payload_size);
}
