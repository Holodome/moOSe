#pragma once

#include <types.h>

int init_net_daemon(void);
void net_daemon_add_frame(const void *data, size_t size);
