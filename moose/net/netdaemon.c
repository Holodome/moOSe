#include <errno.h>
#include <kstdio.h>
#include <mm/kmalloc.h>
#include <net/eth.h>
#include <net/frame.h>
#include <net/inet.h>
#include <net/netdaemon.h>
#include <sched/locks.h>
#include <sched/process.h>
#include <sched/scheduler.h>
#include <string.h>

#define QUEUE_SIZE 32

struct daemon_queue {
    struct net_frame **frames;
    rwlock_t lock;
};

static struct daemon_queue *queue;

__noreturn static void net_daemon_task(void *arg __unused) {
    cpuflags_t flags;
    for (;;) {
        flags = write_lock_irqsave(&queue->lock);
        for (int i = 0; i < QUEUE_SIZE; i++) {
            if (queue->frames[i]) {
                eth_receive_frame(queue->frames[i]);
                release_net_frame(queue->frames[i]);
                queue->frames[i] = NULL;
            }
        }
        write_unlock_irqrestore(&queue->lock, flags);
    }

    exit_current();
}

int init_net_daemon(void) {
    queue = kzalloc(sizeof(*queue) + sizeof(struct net_frame *) * QUEUE_SIZE);
    if (queue == NULL)
        return -ENOMEM;

    queue->frames = (struct net_frame **)(queue + 1);
    init_rwlock(&queue->lock);

    launch_process("net", net_daemon_task, NULL);
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

    cpuflags_t flags = write_lock_irqsave(&queue->lock);

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
