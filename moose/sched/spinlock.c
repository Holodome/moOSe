#include <assert.h>
#include <sched/spinlock.h>

void spin_lock_init(spinlock_t *spinlock) {
    atomic_init(&spinlock->atomic);
}

int spin_trylock(spinlock_t *lock) {
    int old = 0;
    return atomic_try_cmpxchg_acquire(&lock->atomic, &old, 1);
}

void spin_lock(spinlock_t *lock) {
    int old = 0;
    while (!atomic_try_cmpxchg_acquire(&lock->atomic, &old, 1)) spinloop_hint();
}

void spin_unlock(spinlock_t *lock) {
    assert(spin_is_locked(lock));
    atomic_set_release(&lock->atomic, 0);
}

int spin_is_locked(spinlock_t *spinlock) {
    return atomic_read(&spinlock->atomic);
}
