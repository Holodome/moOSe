#pragma once

#include <list.h>
#include <net/device.h>
#include <net/inet.h>

struct ip_route {
    struct ip_addr dst;
    struct ip_addr netmask;
    struct ip_addr gateway;
    struct net_device *dev;

    struct list_head list;
};

int init_routes(void);
struct ip_route *create_route(struct ip_addr *dst_addr, struct ip_addr *netmask,
                              struct ip_addr *gateway, struct net_device *dev);
void destroy_route(struct ip_route *route);

void add_route(struct ip_route *route);
void remove_route(struct ip_route *route);
struct ip_route *route_lookup(struct ip_addr *addr);

void debug_print_route_table(void);
