#pragma once

#include <kthread.h>
#include <types.h>

int init_net_daemon(void);
void free_net_daemon(void);
int net_daemon_add_frame(void *frame, u16 size);
