#include <fs/posix.h>
#include <kstdio.h>
#include <kthread.h>
#include <mm/kmalloc.h>
#include <net/eth.h>
#include <net/frame.h>
#include <net/inet.h>
#include <net/netdaemon.h>
#include <sched/spinlock.h>
#include <string.h>

#define QUEUE_SIZE 32

struct daemon_queue {
    struct net_frame **frames;
    spinlock_t lock;
};

static struct daemon_queue *queue;

__attribute__((noreturn)) static void net_daemon_task(void) {
    u64 flags;
    for (;;) {
        spin_lock_irqsave(&queue->lock, flags);
        for (int i = 0; i < QUEUE_SIZE; i++) {
            if (queue->frames[i]) {
                eth_receive_frame(queue->frames[i]);
                release_net_frame(queue->frames[i]);
                queue->frames[i] = NULL;
            }
        }
        spin_unlock_irqrestore(&queue->lock, flags);
    }
}

int init_net_daemon(void) {
    queue = kzalloc(sizeof(*queue));
    if (queue == NULL)
        return -ENOMEM;

    queue->frames = kzalloc(sizeof(struct net_frame *) * QUEUE_SIZE);
    if (queue->frames == NULL) {
        kfree(queue);
        return -ENOMEM;
    }

    spin_lock_init(&queue->lock);

    if (launch_task(net_daemon_task)) {
        kfree(queue);
        return -1;
    }

    return 0;
}

void net_daemon_add_frame(void *data, size_t size) {
    if (size > ETH_FRAME_MAX_SIZE) {
        kprintf("net daemon add frame error: invalid frame size\n");
        return;
    }

    struct net_frame *frame = get_empty_receive_net_frame();
    if (frame == NULL) {
        kprintf("failed to get empty net frame\n");
        return;
    }

    memcpy(frame->buffer, data, size);
    frame->size = size;

    u64 flags;
    spin_lock_irqsave(&queue->lock, flags);

    for (int i = 0; i < QUEUE_SIZE; i++) {
        if (queue->frames[i] == NULL) {
            queue->frames[i] = frame;
            spin_unlock_irqrestore(&queue->lock, flags);
            return;
        }
    }

    spin_unlock_irqrestore(&queue->lock, flags);

    release_net_frame(frame);
    kprintf("failed to add net frame to daemon queue\n");
}
