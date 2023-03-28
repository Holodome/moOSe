#include <net/netdaemon.h>
#include <net/frame.h>
#include <net/inet.h>
#include <net/eth.h>
#include <mm/kmalloc.h>
#include <kthread.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <kstdio.h>
#include <sched/spinlock.h>

#define QUEUE_SIZE 32

struct queue_entry {
    struct net_frame *frame;
    int is_free;
};

struct daemon_queue {
    struct queue_entry entries[QUEUE_SIZE];
    spinlock_t lock;
};

static struct daemon_queue *queue;

__attribute__((noreturn)) static void net_daemon_task(void) {
    u64 flags;
    for (;;) {
        spin_lock_irqsave(&queue->lock, flags);
        for (int i = 0; i < QUEUE_SIZE; i++) {
            struct queue_entry *entry = &queue->entries[i];
            if (!entry->is_free) {
                eth_receive_frame(entry->frame);

                release_net_frame(entry->frame);
                entry->frame = NULL;
                entry->is_free = 1;
            }
        }
        spin_unlock_irqrestore(&queue->lock, flags);
    }
}

int init_net_daemon(void) {
    queue = kzalloc(sizeof(*queue));
    if (queue == NULL)
        return -ENOMEM;

    for (int i = 0; i < QUEUE_SIZE; i++)
        queue->entries[i].is_free = 1;

    spin_lock_init(&queue->lock);

    if (launch_task(net_daemon_task)) {
        kfree(queue);
        return -1;
    }

    return 0;
}

int net_daemon_add_frame(void *data, size_t size) {
    u64 flags;
    spin_lock_irqsave(&queue->lock, flags);

    expects(size <= ETH_FRAME_MAX_SIZE);
    struct net_frame *frame = get_free_net_frame(RECEIVE_FRAME);
    if (frame == NULL)
        return -ENOMEM;

    memcpy(frame->buffer, data, size);
    frame->size = size;

    for (int i = 0; i < QUEUE_SIZE; i++) {
        struct queue_entry *entry = &queue->entries[i];
        if (entry->is_free) {
            entry->frame = frame;
            entry->is_free = 0;
            spin_unlock_irqrestore(&queue->lock, flags);
            return 0;
        }
    }

    spin_unlock_irqrestore(&queue->lock, flags);

    return -1;
}
