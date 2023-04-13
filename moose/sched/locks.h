#pragma once

#include <arch/atomic.h>
#include <arch/cpu.h>
#include <types.h>

#define __define_lock_irq(_f, _type)                                           \
    static __forceinline void _f##_irq(_type *lock) {                          \
        irq_disable();                                                         \
        _f(lock);                                                              \
    }

#define __define_lock_irqsave(_f, _type)                                       \
    static __nodiscard __forceinline cpuflags_t _f##_irqsave(_type *lock) {    \
        cpuflags_t flags = irq_save();                                         \
        _f(lock);                                                              \
        return flags;                                                          \
    }

#define __define_unlock_irq(_f, _type)                                         \
    static __forceinline void _f##_irq(_type *lock) {                          \
        _f(lock);                                                              \
        irq_enable();                                                          \
    }

#define __define_unlock_irqrestore(_f, _type)                                  \
    static __forceinline void _f##_irqrestore(_type *lock, cpuflags_t flags) { \
        _f(lock);                                                              \
        irq_restore(flags);                                                    \
    }

#define __define_trylock_irq(_f, _type)                                        \
    static __nodiscard __forceinline int _f##_irq(_type *lock) {               \
        irq_disable();                                                         \
        if (_f(lock))                                                          \
            return 1;                                                          \
        irq_enable();                                                          \
        return 0;                                                              \
    }

#define __define_trylock_irqsave(_f, _type)                                    \
    static __nodiscard __forceinline cpuflags_t _f##irqsave(_type *lock) {     \
        cpuflags_t flags = irq_save();                                         \
        if (_f(lock))                                                          \
            return flags;                                                      \
        irq_restore(flags);                                                    \
        return 0;                                                              \
    }

// clang-format off

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

__define_lock_irq(spin_lock, spinlock_t)
__define_lock_irqsave(spin_lock, spinlock_t)
__define_unlock_irq(spin_unlock, spinlock_t)
__define_unlock_irqrestore(spin_unlock, spinlock_t)
__define_trylock_irq(spin_trylock, spinlock_t)
__define_trylock_irqsave(spin_trylock, spinlock_t)

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

__define_lock_irq(read_lock, rwlock_t)
__define_lock_irqsave(read_lock, rwlock_t)
__define_unlock_irq(read_unlock, rwlock_t)
__define_unlock_irqrestore(read_unlock, rwlock_t)

__define_lock_irq(write_lock, rwlock_t)
__define_lock_irqsave(write_lock, rwlock_t)
__define_unlock_irq(write_unlock, rwlock_t)
__define_unlock_irqrestore(write_unlock, rwlock_t)

    // clang-format on
