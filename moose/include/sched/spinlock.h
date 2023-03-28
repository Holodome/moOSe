#pragma once

#include <arch/atomic.h>
#include <arch/cpu.h>
#include <types.h>

typedef struct spinlock {
    atomic_t atomic;
} spinlock_t;

#define SPIN_LOCK_INIT()                                                       \
    { ATOMIC_INIT(0) }

void spin_lock_init(spinlock_t *lock);
int spin_trylock(spinlock_t *lock);
void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);
int spin_is_locked(spinlock_t *lock);

#define spin_lock_irq(_lock)                                                   \
    do {                                                                       \
        irq_disable();                                                         \
        spin_lock(_lock);                                                      \
    } while (0)

#define spin_lock_irqsave(_lock, _flags)                                       \
    do {                                                                       \
        irq_save(_flags);                                                      \
        spin_lock(_lock);                                                      \
    } while (0)

#define spin_unlock_irq(_lock)                                                 \
    do {                                                                       \
        spin_unlock(_lock);                                                    \
        irq_enable();                                                          \
    } while (0)

#define spin_unlock_irqrestore(_lock, _flags)                                  \
    do {                                                                       \
        spin_unlock(_lock);                                                    \
        irq_restore(_flags);                                                   \
    } while (0)

#define spin_trylock_irq(_lock)                                                \
    ({                                                                         \
        irq_disable();                                                         \
        spin_trylock(_lock) ? 1 : ({                                           \
            irq_enable();                                                      \
            0;                                                                 \
        });                                                                    \
    })

#define spin_trylock_irqsave(_lock, _flags)                                    \
    ({                                                                         \
        irq_save(_flags);                                                      \
        spin_trylock(_lock) ? 1 : ({                                           \
            irq_restore(_flags);                                               \
            0;                                                                 \
        });                                                                    \
    })
