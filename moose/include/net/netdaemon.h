#pragma once

#include <net/frame.h>
#include <types.h>

int init_net_daemon(void);
void net_daemon_add_frame(void *data, size_t size);
void print_queue(void);