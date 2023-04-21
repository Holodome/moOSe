#pragma once 

#include <bitops.h>
#include <sched/locks.h>
#include <list.h>

#define MAX_PROCESSES 256

struct scheduler {
    bitmap_t pid_bitmap[BITS_TO_BITMAP(MAX_PROCESSES)];

    struct list_head process_list;
    spinlock_t lock;
};

void init_scheduler(void);
void launch_process(const char *name, void (*function)(void *), void *arg);
void switch_process(struct process *from, struct process *to);
void schedule(void);
