#pragma once

#include <types.h>
#include <kthread.h>

int init_net_daemon(void);
void free_net_daemon(void);
int net_daemon_add_frame(void *frame, u16 size,
                         void (*func)(void *frame, u16 size));
