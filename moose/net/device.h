#pragma once

#include <moose/list.h>
#include <moose/types.h>

struct net_device {
    void *private;
    struct list_head list;
    int (*send)(struct net_device *dev, void *frame, size_t size);
    int (*receive)(struct net_device *dev, void *frame, size_t size);
};
