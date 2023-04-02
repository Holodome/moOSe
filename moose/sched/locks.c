#include <assert.h>
#include <kthread.h>
#include <sched/locks.h>

void spin_lock_init(spinlock_t *spinlock) {
    atomic_init(&spinlock->atomic);
}

int spin_trylock(spinlock_t *lock) {
    int old = 0;
    return atomic_try_cmpxchg_acquire(&lock->atomic, &old, 1);
}

void spin_lock(spinlock_t *lock) {
    int old = 0;
    while (!atomic_try_cmpxchg_acquire(&lock->atomic, &old, 1))
        spinloop_hint();
}

void spin_unlock(spinlock_t *lock) {
    assert(spin_is_locked(lock));
    atomic_set_release(&lock->atomic, 0);
}

int spin_is_locked(spinlock_t *spinlock) {
    return atomic_read(&spinlock->atomic);
}

void rwlock_init(rwlock_t *lock) {
    spin_lock_init(&lock->lock);
    lock->readers = 0;
    lock->writers = 0;
    lock->is_writing = 0;
}

void read_lock(rwlock_t *lock) {
retry:
    spin_lock(&lock->lock);
    if (lock->writers > 0 || lock->is_writing) {
        spin_unlock(&lock->lock);
        spinloop_hint();
        goto retry;
    }
    ++lock->readers;
    spin_unlock(&lock->lock);
}

void read_unlock(rwlock_t *lock) {
    spin_lock(&lock->lock);
    --lock->readers;
    spin_unlock(&lock->lock);
}

void write_lock(rwlock_t *lock) {
    spin_lock(&lock->lock);
    ++lock->writers;
    spin_unlock(&lock->lock);

retry:
    spin_lock(&lock->lock);
    if (lock->readers > 0 || lock->is_writing) {
        spin_unlock(&lock->lock);
        spinloop_hint();
        goto retry;
    }
    --lock->writers;
    lock->is_writing = 1;
    spin_unlock(&lock->lock);
}

void write_unlock(rwlock_t *lock) {
    spin_lock(&lock->lock);
    lock->is_writing = 0;
    spin_unlock(&lock->lock);
}

int recursive_spin_trylock(recursive_spinlock_t *lock) {
    spin_lock(&lock->lock);
    if (lock->count <= 0) {
        lock->pid = current->pid;
        lock->count = 1;
        spin_unlock(&lock->lock);
        return 1;
    }
    if (lock->pid == current->pid) {
        lock->count++;
        spin_unlock(&lock->lock);
        return 1;
    }
    // somebody has the lock and it's not us! return fail
    spin_unlock(&lock->lock);
    return 0;
}

void recursive_spin_lock(recursive_spinlock_t *lock) {
    while (!recursive_spin_trylock(lock))
        spinloop_hint();
}

void recursive_spin_unlock(recursive_spinlock_t *lock) {
    spin_lock(&lock->lock);
    if (--lock->count <= 0)
        lock->count = 0;
    spin_unlock(&lock->lock);
}

void recursive_spinlock_init(recursive_spinlock_t *lock) {
    spin_lock_init(&lock->lock);
    lock->pid = -1;
    lock->count = 0;
}
