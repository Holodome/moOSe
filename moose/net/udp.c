#include <net/udp.h>
#include <net/inet.h>
#include <net/common.h>
#include <net/ip.h>
#include <mm/kmem.h>
#include <endian.h>
#include <kstdio.h>

void udp_send_frame(u8 *dst_ip_addr, u16 src_port, u16 dst_port,
                    void *payload, u16 size) {
    u8 frame[ETH_PAYLOAD_MAX_SIZE];
    struct udp_header *header = (struct udp_header *)frame;
    header->src_port = htobe16(src_port);
    header->dst_port = htobe16(dst_port);
    header->len = sizeof(struct udp_header) + size;
    header->checksum = 0;
    header->checksum = htobe16(inet_checksum(header, header->len));

    memcpy(header + 1, payload, size);
    ipv4_send_frame(dst_ip_addr, IP_PROTOCOL_UDP, frame, header->len);
}

void udp_receive_frame(u8 *frame) {
    struct udp_header *header = (struct udp_header *)frame;
    header->src_port = be16toh(header->src_port);
    header->dst_port = be16toh(header->dst_port);
    header->len = be16toh(header->len);

    void *payload = header + 1;
    u16 payload_size = header->len - sizeof(struct udp_header);

    kprintf("UDP payload:\n");
    debug_print_frame_hexdump(payload, payload_size);
}
