#include <net/arp.h>
#include <net/inet.h>
#include <net/eth.h>
#include <arch/jiffies.h>
#include <mm/kmalloc.h>
#include <mm/kmem.h>
#include <sched/spinlock.h>
#include <endian.h>
#include <kstdio.h>
#include <errno.h>
#include <list.h>

#define ARP_TIMEOUT_MSECS 15000

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
        return -ENOMEM;

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
        if (memcmp(entry->ip_addr, ip_addr, 4) == 0) {
            spin_unlock_irqrestore(&cache->lock, flags);
            return;
        }
    }

    entry = kmalloc(sizeof(struct arp_cache_entry));
    if (entry == NULL) {
        spin_unlock_irqrestore(&cache->lock, flags);
        return;
    }

    memcpy(entry->ip_addr, ip_addr, 4);
    memcpy(entry->mac_addr, mac_addr, 6);
    list_add(&entry->list, &cache->entries);
    spin_unlock_irqrestore(&cache->lock, flags);
}

int arp_get_mac(u8 *ip_addr, u8 *mac_addr) {
    if (arp_cache_get(ip_addr, mac_addr) == 0)
        return 0;

    int err;
    if ((err = arp_send_request(ip_addr)))
        return err;

    int found = 0;
    memcpy(mac_addr, broadcast_mac_addr, 6);
    u64 end = jiffies64_to_msecs(get_jiffies64()) + ARP_TIMEOUT_MSECS;
    while (!(found && memcmp(mac_addr, broadcast_mac_addr, 6) != 0) &&
           jiffies64_to_msecs(get_jiffies64()) < end) {
        found = (arp_cache_get(ip_addr, mac_addr) == 0);
    }

    if (found) return 0;

    return -1;
}

int arp_send_request(u8 *ip_addr) {
    void *frame = kmalloc(sizeof(struct arp_header));
    if (frame == NULL)
        return -ENOMEM;

    struct arp_header *header = frame;
    header->hw_type = htobe16(ETH_HW_TYPE);
    header->protocol_type = htobe16(ETH_TYPE_IPV4);
    header->hw_len = 6;
    header->protocol_len = 4;
    header->operation = htobe16(ARP_REQUEST);
    memcpy(header->src_mac, nic.mac_addr, 6);
    memcpy(header->src_ip, nic.ip_addr, 4);
    memset(header->dst_mac, 0, 6);
    memcpy(header->dst_ip, ip_addr, 4);

    int err;
    if ((err = eth_send_frame(broadcast_mac_addr, ETH_TYPE_ARP,
                             frame, sizeof(struct arp_header))))
        return err;

    kfree(frame);
    return 0;
}

int arp_send_reply(void *frame) {
    struct arp_header *header = frame;

    void *reply_frame = kmalloc(sizeof(struct arp_header));
    if (reply_frame == NULL)
        return -ENOMEM;

    memcpy(reply_frame, frame, sizeof(*reply_frame));

    struct arp_header *reply_header = (struct arp_header *)reply_frame;
    memcpy(reply_header->src_mac, header->dst_mac, 6);
    memcpy(reply_header->dst_mac, nic.mac_addr, 6);
    memcpy(reply_header->src_ip, header->dst_ip, 4);
    memcpy(reply_header->dst_ip, header->src_ip, 4);

    reply_header->operation = htobe16(ARP_REPLY);

    int err;
    if ((err = eth_send_frame(reply_header->dst_mac, ETH_TYPE_ARP,
                             reply_frame, sizeof(struct arp_header))))
        return err;

    kfree(reply_frame);
    return 0;
}

void arp_receive_frame(void *frame) {
    struct arp_header *header = frame;
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

void debug_clear_arp_cache(void) {
    struct arp_cache_entry *entry =
        list_next_or_null(&cache->entries, &cache->entries,
                          struct arp_cache_entry, list);
    while (entry) {
        list_remove(&entry->list);
        kfree(entry);
        entry = list_next_or_null(&cache->entries, &cache->entries,
                                  struct arp_cache_entry, list);
    }
}
