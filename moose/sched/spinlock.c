#include <assert.h>
#include <sched/spinlock.h>

void spin_lock_init(spinlock_t *spinlock) { atomic_init(&spinlock->atomic); }

int spin_trylock(spinlock_t *spinlock) {
    int old = 0;
    return atomic_try_cmpxchg_acquire(&spinlock->atomic, &old, 1);
}

void spin_lock(spinlock_t *spinlock) {
    int old = 0;
    while (!atomic_try_cmpxchg_acquire(&spinlock->atomic, &old, 1))
        spinloop_hint();
}

void spin_unlock(spinlock_t *spinlock) {
    assert(spin_is_locked(spinlock));
    atomic_set(&spinlock->atomic, 0);
}

int spin_is_locked(spinlock_t *spinlock) {
    return atomic_read(&spinlock->atomic);
}

