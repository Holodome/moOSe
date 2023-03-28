#include <sched/rwlock.h>

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
