#include <endian.h>
#include <kstdio.h>
#include <net/frame.h>
#include <net/icmp.h>
#include <net/inet.h>
#include <net/ip.h>
#include <string.h>

#define ICMP_CONTROL_SEQ_BASE 0x0a
#define ICMP_CONTROL_SEQ_SIZE 32

void icmp_send_echo_request(struct net_device *dev, struct net_frame *frame,
                            const u8 *ip_addr) {
    pull_net_frame_head(frame, sizeof(struct icmp_header));
    struct icmp_header *header = frame->head;

    header->type = ICMP_ECHO_REQUEST;
    u8 *control_seq = frame->payload;
    frame->payload_size = ICMP_CONTROL_SEQ_SIZE;
    inc_net_frame_size(frame, frame->payload_size);
    for (int i = 0; i < ICMP_CONTROL_SEQ_SIZE; i++)
        control_seq[i] = ICMP_CONTROL_SEQ_BASE + i;

    header->checksum = htobe16(inet_checksum(header, frame->size));

    memcpy(&frame->icmp_header, frame->head, sizeof(*header));
    frame->transport_kind = TRANSPORT_KIND_ICMP;

    kprintf("icmp request to host ");
    debug_print_ip_addr(ip_addr);

    ipv4_send_frame(dev, frame, ip_addr, IP_PROTOCOL_ICMP);
}

static void icmp_send_echo_reply(struct net_device *dev,
                                 struct net_frame *frame) {
    struct net_frame *reply_frame = get_empty_send_net_frame();
    if (reply_frame == NULL)
        return;

    memcpy(reply_frame->payload, frame->payload, frame->payload_size);

    pull_net_frame_head(reply_frame, sizeof(struct icmp_header));
    struct icmp_header *header = reply_frame->head;

    header->type = ICMP_ECHO_REPLY;
    header->checksum = 0;
    header->checksum =
        htobe16(inet_checksum(reply_frame->head, reply_frame->size));

    memcpy(&reply_frame->icmp_header, reply_frame->head, sizeof(*header));
    reply_frame->transport_kind = TRANSPORT_KIND_ICMP;

    ipv4_send_frame(dev, reply_frame, frame->ipv4_header.src_ip,
                    IP_PROTOCOL_ICMP);
    release_net_frame(reply_frame);
}

void icmp_receive_frame(struct net_device *dev, struct net_frame *frame) {
    struct icmp_header *header = frame->head;

    switch (header->type) {
    case ICMP_ECHO_REQUEST:
        icmp_send_echo_reply(dev, frame);
        break;
    case ICMP_ECHO_REPLY: {
        // check control seq
        push_net_frame_head(frame, sizeof(*header));
        frame->payload = frame->head;
        frame->payload_size = ICMP_CONTROL_SEQ_SIZE;
        u8 *control_seq = frame->payload;
        for (int i = 0; i < ICMP_CONTROL_SEQ_SIZE; i++) {
            if (control_seq[i] != ICMP_CONTROL_SEQ_BASE + i) {
                kprintf("icmp: reply is corrupted, from host ");
                debug_print_ip_addr(frame->ipv4_header.src_ip);
                return;
            }
        }

        kprintf("icmp reply: host is alive ");
        debug_print_ip_addr(frame->ipv4_header.src_ip);
        break;
    }
    default:
        kprintf("unsupported icmp type\n");
    }

    memcpy(&frame->icmp_header, header, sizeof(*header));
    frame->transport_kind = TRANSPORT_KIND_ICMP;
}
