#pragma once

#include <bitops.h>
#include <list.h>
#include <rbtree.h>
#include <sched/locks.h>

#define MAX_PROCESSES 256
#define PRIO_COUNT 40

struct runqueue_rank {
    struct rb_node *root;
    struct rb_node *leftmost;
    size_t count;
};

struct runqueue {
    struct runqueue_rank runqueues[PRIO_COUNT];
};

struct scheduler {
    bitmap_t pid_bitmap[BITS_TO_BITMAP(MAX_PROCESSES)];

    struct list_head process_list;
    spinlock_t lock;
};

void init_scheduler(void);
void launch_process(const char *name, void (*function)(void *), void *arg);
void switch_process(struct process *from, struct process *to);
void schedule(void);
