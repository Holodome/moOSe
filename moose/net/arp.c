#include <net/arp.h>
#include <mm/kmem.h>
#include <net/ip.h>
#include <endian.h>
#include <net/common.h>
#include <drivers/rtl8139.h>
#include <arch/amd64/asm.h>
#include <arch/jiffies.h>
#include <mm/kmalloc.h>
#include <kstdio.h>
#include <list.h>

LIST_HEAD(arp_cache);

struct cache_entry {
    u8 ip_addr[4];
    u8 mac_addr[6];
    struct list_head list;
};

static int arp_cache_get(u8 *ip_addr, u8 *mac_addr) {
    struct cache_entry *entry;
    list_for_each_entry(entry, &arp_cache, list) {
        if (memcmp(entry->ip_addr, ip_addr, 4) == 0) {
            memcpy(entry->mac_addr, mac_addr, 6);
            return 1;
        }
    }

    return 0;
}

__attribute__((unused))
static void arp_cache_add(u8 *ip_addr, u8 *mac_addr) {
    struct cache_entry *entry;
    list_for_each_entry(entry, &arp_cache, list) {
        if (memcmp(entry->ip_addr, ip_addr, 4) == 0)
            return;
    }

    entry = kmalloc(sizeof(*entry));
    if (entry == NULL)
        return;

    memcpy(entry->ip_addr, ip_addr, 4);
    memcpy(entry->mac_addr, mac_addr, 6);
    list_add(&entry->list, &arp_cache);
}

int arp_get_mac(u8 *ip_addr, u8 *mac_addr) {
    if (arp_cache_get(ip_addr, mac_addr))
        return 0;

    arp_send_request(ip_addr);

    int found = 0;
    size_t end = get_jiffies64() + 60;
    while (!found && get_jiffies() < end)
        found = arp_cache_get(ip_addr, mac_addr);

    if (found)
        return 0;

    return -1;
}

void arp_send_request(u8 *ip_addr) {
    u8 frame[sizeof(struct arp_header)];

    struct arp_header *header = (struct arp_header *)frame;
    header->hw_type = htobe16(ETH_HW_TYPE);
    header->protocol_type = htobe16(ETH_TYPE_IPV4);
    header->hw_len = 6;
    header->protocol_len = 4;
    header->operation = htobe16(ARP_REQUEST);
    u8 mac[6] = {0x52, 0x54, 0x00, 0x12, 0x34, 0x56};
    memcpy(header->src_mac, mac, 6);
    memcpy(header->src_ip, rtl8139_ipaddr, 4);
    memset(header->dst_mac, 0, 6);
    memcpy(header->dst_ip, ip_addr, 4);

    u8 broadcast_mac[6];
    memset(broadcast_mac, 0xff, 6);
    rtl8139_send(broadcast_mac, ETH_TYPE_ARP, frame, sizeof(struct arp_header));
}
