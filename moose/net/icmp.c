#include <net/icmp.h>
#include <mm/kmem.h>
#include <net/ip.h>
#include <net/inet.h>
#include <assert.h>
#include <endian.h>
#include <kstdio.h>

void icmp_send_echo_request(u8 *ip_addr) {
    expects(ip_addr != NULL);

    u8 frame[sizeof(struct icmp_header)];
    memset(frame, 0, sizeof(struct icmp_header));

    struct icmp_header *header = (struct icmp_header *)frame;
    header->type = ICMP_ECHO_REQUEST;
    header->checksum = checksum(header, sizeof(struct icmp_header));
    header->checksum = htobe16(header->checksum);
    ipv4_send_frame(ip_addr, IP_PROTOCOL_ICMP, frame, sizeof(struct icmp_header));
}

void icmp_receive_frame(u8 *ip_addr, void *frame, u16 size) {
    assert(frame != NULL);

    struct icmp_header *header = (struct icmp_header *)frame;
    switch (header->type) {
    case ICMP_ECHO_REQUEST:
        header->type = ICMP_ECHO_REPLY;
        header->checksum = 0;
        header->checksum = htobe16(checksum(frame, size));
        ipv4_send_frame(ip_addr, IP_PROTOCOL_ICMP, frame, size);
        break;
    case ICMP_ECHO_REPLY:
        kprintf("host is alive: ");
        debug_print_ip_addr(ip_addr);
        break;
    }
}
