#pragma once

#include <types.h>

int init_net_daemon(void);
int net_daemon_add_frame(void *frame, size_t size);
