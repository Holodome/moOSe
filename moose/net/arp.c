#include <arch/jiffies.h>
#include <endian.h>
#include <fs/posix.h>
#include <kstdio.h>
#include <list.h>
#include <mm/kmalloc.h>
#include <net/arp.h>
#include <net/eth.h>
#include <net/frame.h>
#include <net/inet.h>
#include <sched/rwlock.h>
#include <string.h>

#define ARP_CACHE_SIZE 256
#define ARP_TIMEOUT_MSECS 15000

struct arp_cache_entry {
    u8 ip_addr[4];
    u8 mac_addr[6];
    struct list_head list;
};

struct arp_cache {
    struct list_head entries;
    rwlock_t lock;
};

static LIST_HEAD(free_list);
static struct arp_cache *cache;

int init_arp_cache(void) {
    cache = kmalloc(sizeof(*cache));
    if (cache == NULL)
        return -ENOMEM;

    init_list_head(&cache->entries);
    rwlock_init(&cache->lock);

    for (size_t i = 0; i < ARP_CACHE_SIZE; i++) {
        struct arp_cache_entry *entry = kmalloc(sizeof(*entry));
        if (entry == NULL) {
            destroy_arp_cache();
            return -ENOMEM;
        }

        list_add(&entry->list, &free_list);
    }

    return 0;
}

void destroy_arp_cache(void) {
    struct arp_cache_entry *entry;
    struct arp_cache_entry *temp;
    list_for_each_entry_safe(entry, temp, &free_list, list) {
        kfree(entry);
    }
    init_list_head(&free_list);

    list_for_each_entry_safe(entry, temp, &cache->entries, list) {
        kfree(entry);
    }

    kfree(cache);
}

static int arp_cache_get(const u8 *ip_addr, u8 *mac_addr) {
    u64 flags;
    read_lock_irqsave(&cache->lock, flags);

    struct arp_cache_entry *entry;
    list_for_each_entry(entry, &cache->entries, list) {
        if (memcmp(entry->ip_addr, ip_addr, 4) == 0) {
            memcpy(mac_addr, entry->mac_addr, 6);
            read_unlock_irqrestore(&cache->lock, flags);
            return 0;
        }
    }

    read_unlock_irqrestore(&cache->lock, flags);
    return -1;
}

static void arp_cache_add(const u8 *ip_addr, const u8 *mac_addr) {
    u64 flags;
    write_lock_irqsave(&cache->lock, flags);

    struct arp_cache_entry *entry;
    list_for_each_entry(entry, &cache->entries, list) {
        if (memcmp(entry->ip_addr, ip_addr, 4) == 0) {
            write_unlock_irqrestore(&cache->lock, flags);
            return;
        }
    }

    entry = list_first_or_null(&free_list, struct arp_cache_entry, list);
    if (entry == NULL) {
        kprintf("failed to add arp cache entry\n");
        write_unlock_irqrestore(&cache->lock, flags);
        return;
    }
    list_remove(&entry->list);
    list_add(&entry->list, &cache->entries);

    write_unlock_irqrestore(&cache->lock, flags);

    memcpy(entry->ip_addr, ip_addr, 4);
    memcpy(entry->mac_addr, mac_addr, 6);
}

int arp_get_mac(const u8 *ip_addr, u8 *mac_addr) {
    if (arp_cache_get(ip_addr, mac_addr) == 0)
        return 0;

    struct net_frame *frame = get_empty_send_net_frame();
    if (frame == NULL)
        return -ENOMEM;

    arp_send_request(frame, ip_addr);

    int found = 0;
    memcpy(mac_addr, broadcast_mac_addr, 6);
    u64 end = jiffies64_to_msecs(get_jiffies64()) + ARP_TIMEOUT_MSECS;
    while (!(found && memcmp(mac_addr, broadcast_mac_addr, 6) != 0) &&
           jiffies64_to_msecs(get_jiffies64()) < end) {
        found = (arp_cache_get(ip_addr, mac_addr) == 0);
    }

    release_net_frame(frame);
    return found ? 0 : -1;
}

void arp_send_request(struct net_frame *frame, const u8 *ip_addr) {
    pull_net_frame_head(frame, sizeof(struct arp_header));
    struct arp_header *header = frame->head;

    header->hw_type = htobe16(ETH_HW_TYPE);
    header->protocol_type = htobe16(ETH_TYPE_IPV4);
    header->hw_len = 6;
    header->protocol_len = 4;
    header->operation = htobe16(ARP_REQUEST);
    memcpy(header->src_mac, nic.mac_addr, 6);
    memcpy(header->src_ip, nic.ip_addr, 4);
    memset(header->dst_mac, 0, 6);
    memcpy(header->dst_ip, ip_addr, 4);

    frame->inet_kind = INET_KIND_ARP;
    memcpy(&frame->arp_header, frame->head, sizeof(*header));

    eth_send_frame(frame, broadcast_mac_addr, ETH_TYPE_ARP);
}

static void arp_send_reply(struct net_frame *frame) {
    struct net_frame *reply_frame = get_empty_send_net_frame();
    if (reply_frame == NULL) {
        kprintf("failed to get empty net frame\n");
        return;
    }

    pull_net_frame_head(reply_frame, sizeof(struct arp_header));
    struct arp_header *header = frame->head;
    struct arp_header *reply_header = reply_frame->head;

    memcpy(reply_frame->head, frame->head, sizeof(*reply_header));
    memcpy(reply_header->src_mac, nic.mac_addr, 6);
    memcpy(reply_header->dst_mac, header->src_mac, 6);
    memcpy(reply_header->src_ip, header->dst_ip, 4);
    memcpy(reply_header->dst_ip, header->src_ip, 4);

    reply_header->operation = htobe16(ARP_REPLY);

    memcpy(&reply_frame->arp_header, reply_header, sizeof(*reply_header));
    reply_frame->inet_kind = INET_KIND_ARP;

    eth_send_frame(reply_frame, reply_header->dst_mac, ETH_TYPE_ARP);
    release_net_frame(reply_frame);
}

void arp_receive_frame(struct net_frame *frame) {
    struct arp_header *header = frame->head;
    header->hw_type = be16toh(header->hw_type);
    header->protocol_type = be16toh(header->protocol_type);
    header->operation = be16toh(header->operation);

    // hw type must be Ethernet, protocols ipv4, ipv6 only
    if (header->hw_type != ETH_HW_TYPE ||
        !(header->protocol_type == ETH_TYPE_IPV4 ||
          header->protocol_type == ETH_TYPE_IPV6)) {
        kprintf("arp supports only ethernet link, and ipv4/ipv6\n");
        return;
    }

    memcpy(&frame->arp_header, frame->head, sizeof(*header));
    frame->inet_kind = INET_KIND_ARP;

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
    default:
        kprintf("unsupported arp operation\n");
    }
}

void debug_clear_arp_cache(void) {
    struct arp_cache_entry *entry, *temp;
    list_for_each_entry_safe(entry, temp, &cache->entries, list) {
        list_remove(&entry->list);
        list_add(&entry->list, &free_list);
    }
}
