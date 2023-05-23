#include <mm/kmalloc.h>
#include <net/route.h>
#include <kstdio.h>

static LIST_HEAD(route_table);

int init_routes(void) {
    struct net_device *dev = get_net_device("eth0");
    if (dev == NULL) {
        kprintf("no network device with name eth0\n");
        return -1;
    }

    struct ip_addr dst, netmask, gateway;

    inet_pton(&dst, "192.168.64.1");
    inet_pton(&netmask, "255.255.255.255");
    inet_pton(&gateway, "0.0.0.0");
    struct ip_route *route = create_route(&dst, &netmask, &gateway, dev);
    add_route(route);

    inet_pton(&dst, "0.0.0.0");
    inet_pton(&netmask, "0.0.0.0");
    inet_pton(&gateway, "192.168.64.1");
    route = create_route(&dst, &netmask, &gateway, dev);
    add_route(route);

    inet_pton(&dst, "192.168.64.0");
    inet_pton(&netmask, "255.255.255.0");
    inet_pton(&gateway, "0.0.0.0");
    route = create_route(&dst, &netmask, &gateway, dev);
    add_route(route);

    return 0;
}

static struct ip_route *alloc_route() {
    return kmalloc(sizeof(struct ip_route));
}

static void free_route(struct ip_route *route) {
    kfree(route);
}

struct ip_route *create_route(struct ip_addr *dst_addr, struct ip_addr *netmask,
                              struct ip_addr *gateway, struct net_device *dev) {
    struct ip_route *route = alloc_route();
    if (route == NULL)
        return NULL;

    route->dev = dev;
    route->dst = *dst_addr;
    route->netmask = *netmask;

    if (gateway == NULL)
        route->gateway.addr.bits = 0;
    else
        route->gateway = *gateway;

    return route;
}

void destroy_route(struct ip_route *route) {
    free_route(route);
}

void add_route(struct ip_route *route) {
    struct ip_route *route_entry;
    list_for_each_entry(route_entry, &route_table, list) {
        if (route_entry->netmask.addr.bits > route->netmask.addr.bits)
            break;
    }

    list_add(&route->list, &route_entry->list);
}

void remove_route(struct ip_route *route) {
    list_remove(&route->list);
}

struct ip_route *route_lookup(struct ip_addr *addr) {
    struct ip_route *route;
    list_for_each_entry(route, &route_table, list) {
        if ((addr->addr.bits & route->netmask.addr.bits) == route->dst.addr.bits)
            return route;
    }

    return NULL;
}

void debug_print_route_table(void) {
    kprintf("IP routing table\n");
    kprintf("%-20s%-20s%-20s%-20s", "Destination", "Gateway", "Netmask",
            "Interface");
    struct ip_route *route;
    char buffer[IPV4_ADDR_LEN];
    list_for_each_entry(route, &route_table, list) {
        ip_addr_sprintf(buffer, &route->dst);
        kprintf("%-20s", buffer);
        ip_addr_sprintf(buffer, &route->gateway);
        kprintf("%-20s", buffer);
        ip_addr_sprintf(buffer, &route->netmask);
        kprintf("%-20s", buffer);
        kprintf("%-20s", route->dev->name);
    }
}
