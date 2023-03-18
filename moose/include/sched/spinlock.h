#pragma once

#include <arch/atomic.h>
#include <arch/cpu.h>
#include <types.h>

typedef struct spinlock {
    atomic_t atomic;
} spinlock_t;

#define SPIN_LOCK_INIT()                                                       \
    { ATOMIC_INIT(0) }

void spin_lock_init(spinlock_t *spinlock);
int spin_trylock(spinlock_t *spinlock);
void spin_lock(spinlock_t *spinlock);
void spin_unlock(spinlock_t *spinlock);
int spin_is_locked(spinlock_t *spinlock);

#define spin_lock_irq(_spinlock)                                               \
    do {                                                                       \
        irq_disable();                                                         \
        spin_lock(_spinlock);                                                  \
    } while (0)

#define spin_lock_irqsave(_spinlock, _flags)                                   \
    do {                                                                       \
        irq_save(_flags);                                                      \
        spin_lock(_spinlock);                                                  \
    } while (0)

#define spin_unlock_irq(_spinlock)                                             \
    do {                                                                       \
        irq_enable();                                                          \
        spin_unlock(_spinlock);                                                \
    } while (0)

#define spin_unlock_irqsave(_spinlock, _flags)                                 \
    do {                                                                       \
        spin_unlock(_spinlock);                                                \
        irq_restore(_flags);                                                   \
    } while (0)

#define spin_trylock_irq(_spinlock)                                            \
    ({                                                                         \
        irq_disable();                                                         \
        spin_trylock(_spinlock) ? 1 : ({                                       \
            irq_enable();                                                      \
            0;                                                                 \
        });                                                                    \
    })

#define spin_trylock_irqsave(_spinlock, _flags)                                \
    ({                                                                         \
        irq_save(_flags);                                                      \
        spin_trylock(_spinlock) ? 1 : ({                                       \
            irq_restore(_flags);                                               \
            0;                                                                 \
        });                                                                    \
    })

