#pragma once

#include <moose/arch/atomic.h>

struct process;

typedef struct mutex {
    struct process *holder;
} mutex_t;

#define INIT_MUTEX() {
