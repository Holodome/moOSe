#include <sched/mutex.h>
#include <sched/process.h>

void init_mutex(mutex_t *lock) {
    lock->holder = NULL;
}

void mutex_lock(mutex_t *lock) {
    struct process *expected_holder = NULL;
retry:
    if (__atomic_compare_exchange_n(&lock->holder, &expected_holder,
                                    get_current(), 0, __ATOMIC_ACQUIRE,
                                    __ATOMIC_RELAXED)) {
        return;
    }

    struct process *holder = lock->holder;
    if (holder->state == PROCESS_RUNNING) {
        spinloop_hint();
        goto retry;
    }

    yield();
    goto retry;
}

void mutex_unlock(mutex_t *lock) {
    __atomic_compare_exchange_n(&lock->holder, &lock->holder, NULL, 0,
                                __ATOMIC_RELEASE, __ATOMIC_RELAXED);
}

int mutex_is_locked(mutex_t *lock) {
    return __atomic_load_n(&lock->holder, __ATOMIC_RELAXED) != NULL;
}

