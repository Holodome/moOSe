#include <list.h>
#include <mm/kmalloc.h>
#include <net/interface.h>
#include <string.h>

LIST_HEAD(interfaces);

struct net_interface *create_net_interface(const char *name,
                                           struct net_device *dev) {
    struct net_interface *net_if = kzalloc(sizeof(*net_if));
    if (net_if == NULL)
        return NULL;

    size_t name_size = strlen(name);
    if (name_size > IFNAME_SIZE)
        return NULL;

    memcpy(net_if->name, name, name_size);
    net_if->dev = dev;

    list_add(&net_if->list, &interfaces);

    return net_if;
}

void remove_net_interface(struct net_interface *net_if) {
    list_remove(&net_if->list);
    kfree(net_if);
}
