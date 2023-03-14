#pragma once

#include <types.h>
#include <arch/atomic.h>

typedef struct spinlock {

} spinlock_t;

void spin_lock_init(spinlock_t *spinlock);
