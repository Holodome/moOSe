#include <assert.h>
#include <ctype.h>
#include <drivers/rtl8139.h>
#include <endian.h>
#include <kstdio.h>
#include <net/arp.h>
#include <net/device.h>
#include <net/frame.h>
#include <net/inet.h>
#include <net/netdaemon.h>
#include <string.h>

struct ip_addr gateway_ip_addr;
struct ip_addr dns_ip_addr;
struct ip_addr local_net_ip_addr;
struct ip_addr local_net_mask;
struct mac_addr broadcast_mac_addr;
struct mac_addr null_mac_addr;

static int test_inet(struct net_device *dev) {
    struct mac_addr mac_addr;
    for (int i = 0; i < 5; i++) {
        if (arp_get_mac(dev, &gateway_ip_addr, &mac_addr)) {
            kprintf("can't find mac for this ip address\n");
            return -1;
        }
        kprintf("gateway ");
        debug_print_mac_addr(&mac_addr);
        debug_clear_arp_cache();
    }

    if (arp_get_mac(dev, &dns_ip_addr, &mac_addr)) {
        kprintf("can't find mac for this ip address\n");
        return -1;
    }
    kprintf("dns ");
    debug_print_mac_addr(&mac_addr);

    struct net_frame *frame = get_empty_send_net_frame();
    icmp_send_echo_request(dev, frame, &gateway_ip_addr);
    release_net_frame(frame);

    frame = get_empty_send_net_frame();
    char *message = "Hello world!";
    memcpy(frame->payload, message, strlen(message));
    frame->payload_size = strlen(message);
    frame->size = frame->payload_size;

    udp_send_frame(dev, frame, &gateway_ip_addr, 80, 80);
    release_net_frame(frame);

    return 0;
}

int init_inet(void) {
    inet_pton(&gateway_ip_addr, "10.0.2.2");
    inet_pton(&dns_ip_addr, "10.0.2.3");
    inet_pton(&local_net_ip_addr, "10.0.2.0");
    inet_pton(&local_net_mask, "255.255.255.0");
    for (size_t i = 0; i < 6; i++)
        broadcast_mac_addr.octets[i] = 0xff;
    for (size_t i = 0; i < 6; i++)
        null_mac_addr.octets[i] = 0x0;

    debug_print_ip_addr(&gateway_ip_addr);

    struct net_device *rtl8139 = create_rtl8139();
    expects(rtl8139 != NULL);
    expects(rtl8139->ops->open(rtl8139) == 0);
    expects(init_net_frames() == 0);
    expects(init_arp_cache() == 0);
    expects(init_net_daemon() == 0);

    kprintf("rtl8139 ");
    debug_print_mac_addr(&rtl8139->mac_addr);
    expects(test_inet(rtl8139) == 0);

    return 0;
}

u16 inet_checksum(const void *data, size_t size) {
    const u16 *ptr = data;
    u32 sum = 0;

    while (size > 1) {
        sum += htobe16(*ptr++);
        size -= 2;
    }

    if (size > 0)
        sum += htobe16(*(u8 *)data << 16);

    sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;
}

void debug_print_mac_addr(const struct mac_addr *mac_addr) {
    kprintf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", mac_addr->octets[0],
            mac_addr->octets[1], mac_addr->octets[2], mac_addr->octets[3],
            mac_addr->octets[4], mac_addr->octets[5]);
}

void debug_print_ip_addr(const struct ip_addr *ip_addr) {
    kprintf("IP: %d.%d.%d.%d\n", ip_addr->addr.octets[0],
            ip_addr->addr.octets[1], ip_addr->addr.octets[2],
            ip_addr->addr.octets[3]);
}

void debug_print_frame_hexdump(const void *frame, size_t size) {
    const u8 *data = frame;
    size_t dump_lines = size / 16;
    if (size % 16 != 0)
        dump_lines++;

    for (size_t i = 0; i < dump_lines; i++) {
        kprintf("%06lx ", i * 16);
        for (u8 j = 0; j < 16; j++)
            kprintf("%02x ", data[i * 16 + j]);
        kprintf("\n");
    }
}

int ip_addr_equals(const struct ip_addr *a, const struct ip_addr *b) {
    return a->addr.bits == b->addr.bits;
}

int mac_addr_equals(const struct mac_addr *a, const struct mac_addr *b) {
    for (size_t i = 0; i < 6; i++)
        if (a->octets[i] != b->octets[i])
            return 0;

    return 1;
}

struct ip_addr *copy_ip_addr(struct ip_addr *dst, const struct ip_addr *src) {
    dst->addr.bits = src->addr.bits;
    return dst;
}

struct mac_addr *copy_mac_addr(struct mac_addr *dst,
                               const struct mac_addr *src) {
    for (size_t i = 0; i < 6; i++)
        dst->octets[i] = src->octets[i];
    return dst;
}

int inet_pton(struct ip_addr *addr, const char *str) {
    for (size_t i = 0; i < 4; i++) {
        u8 octet = 0;
        while (*str) {
            if (isdigit(*str)) {
                octet *= 10;
                octet += *str - '0';
            } else {
                str++;
                break;
            }
            str++;
        }
        addr->addr.octets[i] = octet;
    }

    return 0;
}

int inet_ntop(char *str, const struct ip_addr *addr) {
    snprintf(str, IPV4_ADDR_LEN, "%u.%u.%u.%u", addr->addr.octets[0],
             addr->addr.octets[1], addr->addr.octets[2], addr->addr.octets[3]);
    return 0;
}

int is_subnet_ip_addr(const struct ip_addr *ip_addr) {
    struct ip_addr temp;
    copy_ip_addr(&temp, ip_addr);
    for (u8 i = 0; i < 4; i++)
        temp.addr.octets[i] &= local_net_mask.addr.octets[i];

    return ip_addr_equals(&temp, &local_net_ip_addr);
}
