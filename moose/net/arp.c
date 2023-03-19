#include <net/arp.h>
#include <net/common.h>
#include <net/inet.h>
#include <net/eth.h>
#include <arch/jiffies.h>
#include <mm/kmalloc.h>
#include <mm/kmem.h>
#include <sched/spinlock.h>
#include <endian.h>
#include <kstdio.h>
#include <list.h>

struct arp_cache_entry {
    u8 ip_addr[4];
    u8 mac_addr[6];
    struct list_head list;
};

struct arp_cache {
    struct list_head entries;
    spinlock_t lock;
};

static struct arp_cache *cache;

int init_arp_cache(void) {
    cache = kmalloc(sizeof(*cache));
    if (cache == NULL)
        return -1;

    init_list_head(&cache->entries);
    spin_lock_init(&cache->lock);

    return 0;
}

static int arp_cache_get(u8 *ip_addr, u8 *mac_addr) {
    u64 flags;
    spin_lock_irqsave(&cache->lock, flags);

    struct arp_cache_entry *entry;
    list_for_each_entry(entry, &cache->entries, list) {
        if (memcmp(entry->ip_addr, ip_addr, 4) == 0) {
            memcpy(mac_addr, entry->mac_addr, 6);
            spin_unlock_irqrestore(&cache->lock, flags);
            return 0;
        }
    }

    spin_unlock_irqrestore(&cache->lock, flags);
    return -1;
}

static void arp_cache_add(u8 *ip_addr, u8 *mac_addr) {
    u64 flags;
    spin_lock_irqsave(&cache->lock, flags);

    struct arp_cache_entry *entry;
    list_for_each_entry(entry, &cache->entries, list) {
        if (memcmp(entry->ip_addr, ip_addr, 4) == 0)
            goto error;
    }

    entry = kmalloc(sizeof(*entry));
    if (entry == NULL)
        goto error;

    memcpy(entry->ip_addr, ip_addr, 4);
    memcpy(entry->mac_addr, mac_addr, 6);
    list_add(&entry->list, &cache->entries);

error:
    spin_unlock_irqrestore(&cache->lock, flags);
}

int arp_get_mac(u8 *ip_addr, u8 *mac_addr) {
    if (arp_cache_get(ip_addr, mac_addr) == 0)
        return 0;

    arp_send_request(ip_addr);

    int found = 0;
    memcpy(mac_addr, broadcast_mac_addr, 6);
    u64 end = jiffies64_to_msecs(get_jiffies64()) + 5 * 1000;
    while ((!found || memcmp(mac_addr, broadcast_mac_addr, 6) == 0) &&
           jiffies64_to_msecs(get_jiffies64()) < end) {
        found = (arp_cache_get(ip_addr, mac_addr) == 0);
    }

    if (found) return 0;

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
    memcpy(header->src_mac, nic.mac_addr, 6);
    memcpy(header->src_ip, nic.ip_addr, 4);
    memset(header->dst_mac, 0, 6);
    memcpy(header->dst_ip, ip_addr, 4);

    eth_send_frame(broadcast_mac_addr, ETH_TYPE_ARP,
                   frame, sizeof(struct arp_header));
}

void arp_send_reply(void *frame) {
    struct arp_header *header = (struct arp_header *)frame;

    u8 reply_frame[sizeof(struct arp_header)];
    memcpy(reply_frame, frame, sizeof(*reply_frame));

    struct arp_header *reply_header = (struct arp_header *)reply_frame;
    memcpy(reply_header->src_mac, header->dst_mac, 6);
    memcpy(reply_header->dst_mac, nic.mac_addr, 6);
    memcpy(reply_header->src_ip, header->dst_ip, 4);
    memcpy(reply_header->dst_ip, header->src_ip, 4);

    reply_header->operation = htobe16(ARP_REPLY);

    eth_send_frame(reply_header->dst_mac, ETH_TYPE_ARP,
                   reply_frame, sizeof(struct arp_header));
}

void arp_receive_frame(void *frame) {
    struct arp_header *header = (struct arp_header *)frame;
    header->hw_type = be16toh(header->hw_type);
    header->protocol_type = be16toh(header->protocol_type);
    header->operation = be16toh(header->operation);

    // hw type must be Ethernet, protocols ipv4, ipv6 only
    if (header->hw_type != ETH_HW_TYPE ||
        !(header->protocol_type == ETH_TYPE_IPV4 ||
        header->protocol_type == ETH_TYPE_IPV6))
        return;

    switch (header->operation) {
    case ARP_REQUEST:
        if (memcmp(header->dst_ip, nic.ip_addr, 4) == 0) {
            arp_cache_add(header->src_ip, header->src_mac);
            arp_send_reply(frame);
        }
        break;
    case ARP_REPLY:
        arp_cache_add(header->src_ip, header->src_mac);
        break;
    }
}
