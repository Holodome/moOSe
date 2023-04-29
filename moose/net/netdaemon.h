#pragma once

#include <types.h>
#include <net/device.h>

int init_net_daemon(void);
void net_daemon_add_frame(struct net_device *dev, const void *data, size_t size);
