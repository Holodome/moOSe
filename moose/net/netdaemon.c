#include <errno.h>
#include <kstdio.h>
#include <mm/kmalloc.h>
#include <net/device.h>
#include <net/eth.h>
#include <net/frame.h>
#include <net/inet.h>
#include <net/netdaemon.h>
#include <sched/locks.h>
#include <sched/process.h>
#include <string.h>

#define QUEUE_SIZE 32

struct daemon_queue_entry {
    struct net_frame *frame;
    struct net_device *dev;
    int free;
};

struct daemon_queue {
    struct daemon_queue_entry *entries;
    rwlock_t lock;
};

static struct daemon_queue *queue;

__noreturn static void net_daemon_task(void *arg __unused) {
    cpuflags_t flags;
    struct daemon_queue_entry *entry;
    for (;;) {
        flags = write_lock_irqsave(&queue->lock);
        for (int i = 0; i < QUEUE_SIZE; i++) {
            entry = &queue->entries[i];
            if (!entry->free) {
                eth_receive_frame(entry->dev, entry->frame);
                release_net_frame(entry->frame);
                entry->free = 1;
            }
        }
        write_unlock_irqrestore(&queue->lock, flags);
    }

    exit_current();
}

int init_net_daemon(void) {
    queue = kzalloc(sizeof(*queue) +
                    sizeof(struct daemon_queue_entry) * QUEUE_SIZE);
    if (queue == NULL)
        return -ENOMEM;

    queue->entries = (struct daemon_queue_entry *)(queue + 1);
    for (size_t i = 0; i < QUEUE_SIZE; i++)
        queue->entries[i].free = 1;

    init_rwlock(&queue->lock);

    launch_process("net", net_daemon_task, NULL);
    return 0;
}

void net_daemon_add_frame(struct net_device *dev, const void *data,
                          size_t size) {
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

    struct daemon_queue_entry *entry;
    for (int i = 0; i < QUEUE_SIZE; i++) {
        entry = &queue->entries[i];
        if (entry->free) {
            entry->frame = frame;
            entry->dev = dev;
            entry->free = 0;
            write_unlock_irqrestore(&queue->lock, flags);
            return;
        }
    }

    write_unlock_irqrestore(&queue->lock, flags);

    release_net_frame(frame);
    kprintf("failed to add net frame to daemon queue\n");
}
