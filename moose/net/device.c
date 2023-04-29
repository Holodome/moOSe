#include <net/device.h>
#include <mm/kmalloc.h>
#include <string.h>
#include <kstdio.h>

static LIST_HEAD(net_device_list);

struct net_device *create_net_device(const char *name) {
    struct net_device *dev = kmalloc(sizeof(struct net_device));
    if (dev == NULL)
        return NULL;

    size_t name_size = strlen(name);
    if (name_size > IFNAME_SIZE) {
        kprintf("interface name was truncated\n");
    }

    strlcpy(dev->name, name, IFNAME_SIZE);
    list_add(&dev->list, &net_device_list);

    return dev;
}

void destroy_net_device(struct net_device *dev) {
    list_remove(&dev->list);
    kfree(dev);
}
