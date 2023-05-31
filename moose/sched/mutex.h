#pragma once

#include <moose/arch/atomic.h>

struct process;

typedef struct mutex {
    struct process *holder;
} mutex_t;

#define INIT_MUTEX()                                                           \
    { NULL }

void init_mutex(mutex_t *lock);
void mutex_lock(mutex_t *lock);
void mutex_unlock(mutex_t *lock);
int mutex_is_locked(mutex_t *lock);