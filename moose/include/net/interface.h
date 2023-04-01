#pragma once

#include <net/device.h>

#define IFNAME_SIZE 16

struct net_interface {
    u8 mac_addr[6];
    u8 ip_addr[4];
    u8 broadcast_ip_addr[4];
    u8 net_mask[4];
    char name[IFNAME_SIZE + 1];

    struct net_device *dev;
    struct list_head list;
};

struct net_interface *create_net_interface(const char *name,
                                           struct net_device *dev);
void remove_net_interface(struct net_interface *net_if);
