#include <net/icmp.h>
#include <mm/kmem.h>
#include <net/ip.h>
#include <net/inet.h>
#include <assert.h>
#include <endian.h>
#include <kstdio.h>

#define ICMP_CONTROL_SEQ_BASE 0x0a
#define ICMP_CONTROL_SEQ_SIZE 32

void icmp_send_echo_request(u8 *ip_addr) {
    expects(ip_addr != NULL);

    u8 frame[sizeof(struct icmp_header) + ICMP_CONTROL_SEQ_SIZE];
    memset(frame, 0, sizeof(struct icmp_header));

    u16 frame_size = sizeof(struct icmp_header) + ICMP_CONTROL_SEQ_SIZE;

    struct icmp_header *header = (struct icmp_header *)frame;
    header->type = ICMP_ECHO_REQUEST;
    u8 *control_seq = (u8 *)(header + 1);
    for (int i = 0; i < ICMP_CONTROL_SEQ_SIZE; i++)
        control_seq[i] = ICMP_CONTROL_SEQ_BASE + i;

    header->checksum = inet_checksum(header, frame_size);
    header->checksum = htobe16(header->checksum);

    kprintf("icmp request to host ");
    debug_print_ip_addr(ip_addr);
    ipv4_send_frame(ip_addr, IP_PROTOCOL_ICMP, frame, frame_size);
}

void icmp_receive_frame(u8 *ip_addr, void *frame, u16 size) {
    assert(frame != NULL);

    struct icmp_header *header = (struct icmp_header *)frame;
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
    }
}
