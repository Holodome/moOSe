#pragma once

#include <net/device.h>
#include <types.h>

int init_net_daemon(void);
void net_daemon_add_frame(struct net_device *dev, const void *data,
                          size_t size);
