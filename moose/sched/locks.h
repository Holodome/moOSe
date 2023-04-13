#pragma once

#include <arch/atomic.h>
#include <arch/cpu.h>
#include <types.h>

#define __lock_irq(_f, _lock)                                                  \
    do {                                                                       \
        irq_disable();                                                         \
        _f(_lock);                                                             \
    } while (0)

#define __lock_irqsave(_f, _lock, _flags)                                      \
    do {                                                                       \
        irq_save(_flags);                                                      \
        _f(_lock);                                                             \
    } while (0)

#define __unlock_irq(_f, _lock)                                                \
    do {                                                                       \
        _f(_lock);                                                             \
        irq_enable();                                                          \
    } while (0)

#define __unlock_irqrestore(_f, _lock, _flags)                                 \
    do {                                                                       \
        _f(_lock);                                                             \
        irq_restore(_flags);                                                   \
    } while (0)

#define __trylock_irq(_f, _lock)                                               \
    ({                                                                         \
        irq_disable();                                                         \
        _f(_lock) ? 1 : ({                                                     \
            irq_enable();                                                      \
            0;                                                                 \
        });                                                                    \
    })

#define __trylock_irqsave(_f, _lock, _flags)                                   \
    ({                                                                         \
        irq_save(_flags);                                                      \
        _f(_lock) ? 1 : ({                                                     \
            irq_restore(_flags);                                               \
            0;                                                                 \
        });                                                                    \
    })

typedef struct spinlock {
    atomic_t atomic;
} spinlock_t;

#define INIT_SPIN_LOCK()                                                       \
    { INIT_ATOMIC(0) }

void init_spin_lock(spinlock_t *lock);
int spin_trylock(spinlock_t *lock);
void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);
int spin_is_locked(spinlock_t *lock);

#define spin_lock_irq(_lock) __lock_irq(spin_lock, _lock)
#define spin_lock_irqsave(_lock, _flags)                                       \
    __lock_irqsave(spin_lock, _lock, _flags)
#define spin_unlock_irq(_lock) __unlock_irq(spin_unlock, _lock)
#define spin_unlock_irqrestore(_lock, _flags)                                  \
    __unlock_irqrestore(spin_unlock, _lock, _flags)
#define spin_trylock_irq(_lock) __trylock_irq(spin_trylock, _lock)
#define spin_trylock_irqsave(_lock, _flags)                                    \
    __trylock_irqsave(spin_trylock, _lock, _flags)

typedef struct rwlock {
    spinlock_t lock;
    int readers;
    int writers;
    int is_writing;
} rwlock_t;

#define INIT_RWLOCK()                                                          \
    { INIT_SPINLOCK(), 0, 0, 0 }

void init_rwlock(rwlock_t *lock);

void read_lock(rwlock_t *lock);
void read_unlock(rwlock_t *lock);

void write_lock(rwlock_t *lock);
void write_unlock(rwlock_t *lock);

#define read_lock_irq(_lock) __lock_irq(read_lock, _lock)
#define read_lock_irqsave(_lock, _flags)                                       \
    __lock_irqsave(read_lock, _lock, _flags)
#define read_unlock_irq(_lock) __unlock_irq(read_unlock, _lock)
#define read_unlock_irqrestore(_lock, _flags)                                  \
    __unlock_irqrestore(read_unlock, _lock, _flags)

#define write_lock_irq(_lock) __lock_irq(write_lock, _lock)
#define write_lock_irqsave(_lock, _flags)                                      \
    __lock_irqsave(write_lock, _lock, _flags)
#define write_unlock_irq(_lock) __unlock_irq(write_unlock, _lock)
#define write_unlock_irqrestore(_lock, _flags)                                 \
    __unlock_irqrestore(write_unlock, _lock, _flags)
