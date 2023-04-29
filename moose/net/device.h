#pragma once

#include <bitops.h>
#include <list.h>
#include <types.h>

#define IFNAME_SIZE 16

#define IF_LOOPBACK BIT(0)

struct net_device;

struct net_device_ops {
    int (*open)(struct net_device *dev);
    int (*close)(struct net_device *dev);
    void (*set_mac_addr)(struct net_device *dev, u8 *mac_addr);
    int (*transmit)(struct net_device *dev, const void *frame, size_t size);
};

struct net_device_stats {
    size_t rx_frames;
    size_t tx_frames;
    size_t rx_bytes;
    size_t tx_bytes;
    size_t rx_errors;
    size_t tx_errors;
};

struct net_device {
    void *private;
    u16 flags;

    u8 mac_addr[6];
    u8 ip_addr[4];
    u8 broadcast_ip_addr[4];
    u8 net_mask[4];

    char name[IFNAME_SIZE + 1];
    struct list_head list;
    const struct net_device_ops *ops;
    struct net_device_stats stats;
};

struct net_device *create_net_device(const char *name);
void destroy_net_device(struct net_device *dev);
