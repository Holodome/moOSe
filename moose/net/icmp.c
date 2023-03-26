#include <net/icmp.h>
#include <net/ip.h>
#include <net/inet.h>
#include <net/frame.h>
#include <assert.h>
#include <endian.h>
#include <kstdio.h>
#include <errno.h>
#include <mm/kmalloc.h>

#define ICMP_CONTROL_SEQ_BASE 0x0a
#define ICMP_CONTROL_SEQ_SIZE 32

int icmp_send_echo_request(u8 *ip_addr) {
    u16 frame_size = sizeof(struct icmp_header) + ICMP_CONTROL_SEQ_SIZE;
    struct net_frame *frame = alloc_net_frame();
    if (frame == NULL)
        return -ENOMEM;

    frame->size = frame_size;

    struct icmp_header *header = frame->data;
    header->type = ICMP_ECHO_REQUEST;
    u8 *control_seq = (u8 *)(header + 1);
    for (int i = 0; i < ICMP_CONTROL_SEQ_SIZE; i++)
        control_seq[i] = ICMP_CONTROL_SEQ_BASE + i;

    header->checksum = htobe16(inet_checksum(header, frame_size));

    kprintf("icmp request to host ");
    debug_print_ip_addr(ip_addr);

    int err = ipv4_send_frame(ip_addr, IP_PROTOCOL_ICMP,
                              frame->data, frame->size);
    if (err) return err;

    free_net_frame(frame);
    return 0;
}

void icmp_receive_frame(u8 *ip_addr, void *frame, size_t size) {
    expects(size <= ETH_PAYLOAD_MAX_SIZE);

    struct icmp_header *header = frame;

    switch (header->type) {
    case ICMP_ECHO_REQUEST:
        header->type = ICMP_ECHO_REPLY;
        header->checksum = 0;
        header->checksum = htobe16(inet_checksum(frame, size));
        ipv4_send_frame(ip_addr, IP_PROTOCOL_ICMP, frame, size);
        break;
    case ICMP_ECHO_REPLY: {
        // check control seq
        u8 *control_seq = (u8 *)(header + 1);
        for (int i = 0; i < ICMP_CONTROL_SEQ_SIZE; i++) {
            if (control_seq[i] != ICMP_CONTROL_SEQ_BASE + i) {
                kprintf("icmp: reply is corrupted, from host ");
                debug_print_ip_addr(ip_addr);
                return;
            }
        }

        kprintf("icmp reply: host is alive ");
        debug_print_ip_addr(ip_addr);
        break;
    }
    default:
        kprintf("unsupported icmp type\n");
    }
}
