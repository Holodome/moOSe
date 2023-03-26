#pragma once

#include <types.h>
#include <list.h>

struct net_frame {
    void *data;
    size_t size;
    struct list_head list;
};

int init_net_frames(void);
struct net_frame *alloc_net_frame(void);
void free_net_frame(struct net_frame *frame);
