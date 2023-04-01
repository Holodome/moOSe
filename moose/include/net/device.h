#pragma once

#include <list.h>
#include <types.h>

struct net_device {
    void *private;
    u8 mac_addr[6];
    struct list_head list;
    int (*send)(struct net_device *dev, const void *frame, size_t size);
};
