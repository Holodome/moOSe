#pragma once

#include <list.h>
#include <net/inet.h>
#include <net/device.h>

struct ip_route {
    struct ip_addr dst;
    struct ip_addr netmask;
    struct ip_addr gateway;
    struct net_device *dev;

    struct list_head list;
};
