#include <errno.h>
#include <fs/posix.h>
#include <kstdio.h>
#include <kthread.h>
#include <mm/kmalloc.h>
#include <net/eth.h>
#include <net/frame.h>
#include <net/inet.h>
#include <net/netdaemon.h>
#include <sched/rwlock.h>
#include <string.h>

#define QUEUE_SIZE 32

struct daemon_queue {
    struct net_frame **frames;
    rwlock_t lock;
};

static struct daemon_queue *queue;

__noreturn __used static void net_daemon_task(void) {
    cpuflags_t flags;
    for (;;) {
        write_lock_irqsave(&queue->lock, flags);
        for (int i = 0; i < QUEUE_SIZE; i++) {
            if (queue->frames[i]) {
                eth_receive_frame(queue->frames[i]);
                release_net_frame(queue->frames[i]);
                queue->frames[i] = NULL;
            }
        }
        write_unlock_irqrestore(&queue->lock, flags);
    }
}

int init_net_daemon(void) {
    queue = kzalloc(sizeof(*queue) + sizeof(struct net_frame *) * QUEUE_SIZE);
    if (queue == NULL)
        return -ENOMEM;

    queue->frames = (struct net_frame **)(queue + 1);
    rwlock_init(&queue->lock);

    if (launch_task("net", net_daemon_task)) {
        kfree(queue);
        return -1;
    }

    return 0;
}

void net_daemon_add_frame(const void *data, size_t size) {
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
    write_lock_irqsave(&queue->lock, flags);

    for (int i = 0; i < QUEUE_SIZE; i++) {
        if (queue->frames[i] == NULL) {
            queue->frames[i] = frame;
            write_unlock_irqrestore(&queue->lock, flags);
            return;
        }
    }

    write_unlock_irqrestore(&queue->lock, flags);

    release_net_frame(frame);
    kprintf("failed to add net frame to daemon queue\n");
}
