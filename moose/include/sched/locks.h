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

#define SPIN_LOCK_INIT()                                                       \
    { ATOMIC_INIT(0) }

void spin_lock_init(spinlock_t *lock);
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

typedef struct recursive_spinlock {
    spinlock_t lock;
    int pid;
    int count;
} recursive_spinlock_t;

void recursive_spin_lock_init(recursive_spinlock_t *lock);
int recursive_spin_trylock(recursive_spinlock_t *lock);
void recursive_spin_lock(recursive_spinlock_t *lock);
void recursive_spin_unlock(recursive_spinlock_t *lock);
int recursive_spin_is_locked(recursive_spinlock_t *lock);

#define recursive_spin_lock_irq(_lock) __lock_irq(recursive_spin_lock, _lock)
#define recursive_spin_lock_irqsave(_lock, _flags)                             \
    __lock_irqsave(spin_lock, _lock, _flags)
#define recursive_spin_unlock_irq(_lock)                                       \
    __unlock_irq(recursive_spin_unlock, _lock)
#define recursive_spin_unlock_irqrestore(_lock, _flags)                        \
    __unlock_irqrestore(recursive_spin_unlock, _lock, _flags)
#define recursive_spin_trylock_irq(_lock)                                      \
    __trylock_irq(recursive_spin_trylock, _lock)
#define recursive_spin_trylock_irqsave(_lock, _flags)

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

