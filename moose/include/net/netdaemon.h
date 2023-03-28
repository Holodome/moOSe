#pragma once

#include <types.h>
#include <net/frame.h>

int init_net_daemon(void);
int net_daemon_add_frame(void *data, size_t size);
void print_queue(void);