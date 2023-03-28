#pragma once

#include <arch/atomic.h>
#include <sched/spinlock.h>
#include <types.h>

typedef struct rwlock {
    spinlock_t lock;
    int readers;
    int writers;
    int is_writing;
} rwlock_t;

#define RWLOCK_INIT()                                                          \
    { SPINLOCK_INIT(), 0, 0, 0 }

void rwlock_init(rwlock_t *lock);

void read_lock(rwlock_t *lock);
void read_unlock(rwlock_t *lock);

void write_lock(rwlock_t *lock);
void write_unlock(rwlock_t *lock);

#define read_lock_irq(_lock)                                                   \
    do {                                                                       \
        irq_disable();                                                         \
        read_lock(_lock);                                                      \
    } while (0)

#define read_lock_irqsave(_lock, _flags)                                       \
    do {                                                                       \
        irq_save(_flags);                                                      \
        read_lock(_lock);                                                      \
    } while (0)

#define read_unlock_irq(_lock)                                                 \
    do {                                                                       \
        read_unlock(_lock);                                                    \
        irq_enable();                                                          \
    } while (0)

#define read_unlock_irqrestore(_lock, _flags)                                  \
    do {                                                                       \
        read_unlock(_lock);                                                    \
        irq_restore(_flags);                                                   \
    } while (0)

#define write_lock_irq(_lock)                                                  \
    do {                                                                       \
        irq_disable();                                                         \
        write_lock(_lock);                                                     \
    } while (0)

#define write_lock_irqsave(_lock, _flags)                                      \
    do {                                                                       \
        irq_save(_flags);                                                      \
        write_lock(_lock);                                                     \
    } while (0)

#define write_unlock_irq(_lock)                                                \
    do {                                                                       \
        write_unlock(_lock);                                                   \
        irq_enable();                                                          \
    } while (0)

#define write_unlock_irqrestore(_lock, _flags)                                 \
    do {                                                                       \
        write_unlock(_lock);                                                   \
        irq_restore(_flags);                                                   \
    } while (0)

