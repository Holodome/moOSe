#include <net/udp.h>
#include <net/inet.h>
#include <net/ip.h>
#include <mm/kmem.h>
#include <mm/kmalloc.h>
#include <string.h>
#include <endian.h>
#include <kstdio.h>
#include <errno.h>

int udp_send_frame(u8 *dst_ip_addr, u16 src_port, u16 dst_port,
                   void *payload, size_t size) {
    size_t frame_size = sizeof(struct udp_header) + size;
    void *frame = kmalloc(frame_size);
    if (frame == NULL)
        return -ENOMEM;

    struct udp_header *header = frame;
    header->src_port = htobe16(src_port);
    header->dst_port = htobe16(dst_port);
    header->len = frame_size;
    header->checksum = 0;
    header->checksum = htobe16(inet_checksum(header, header->len));

    memcpy(header + 1, payload, size);

    int err;
    if ((err = ipv4_send_frame(dst_ip_addr, IP_PROTOCOL_UDP,
                               frame, header->len)))
        return err;

    kfree(frame);
    return 0;
}

void udp_receive_frame(void *frame) {
    struct udp_header *header = frame;
    header->src_port = be16toh(header->src_port);
    header->dst_port = be16toh(header->dst_port);
    header->len = be16toh(header->len);

    void *payload = header + 1;
    u16 payload_size = header->len - sizeof(struct udp_header);

    kprintf("UDP payload:\n");
    debug_print_frame_hexdump(payload, payload_size);
}
