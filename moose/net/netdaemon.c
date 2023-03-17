#include <net/netdaemon.h>
#include <net/common.h>
#include <net/eth.h>
#include <mm/kmalloc.h>
#include <mm/kmem.h>
#include <kthread.h>
#include <endian.h>

#define QUEUE_SIZE 10
#define BUFFER_SIZE ETH_FRAME_MAX_SIZE

struct queue_entry {
    u8 *buffer;
    u16 size;
};

static struct {
    struct queue_entry entries[QUEUE_SIZE];

    struct queue_entry *head;
    struct queue_entry *tail;
    u16 count;
} daemon_queue;

__attribute__((noreturn)) static void net_daemon_task(void) {
    for (;;) {
        while (daemon_queue.count) {
            struct queue_entry *entry = daemon_queue.head;

            eth_receive_frame(entry->buffer, entry->size);

            if (daemon_queue.head + 1 >= daemon_queue.entries + QUEUE_SIZE)
                daemon_queue.head = daemon_queue.entries;
            else daemon_queue.head++;
            daemon_queue.count--;
        }
    }
}

int init_net_daemon(void) {
    for (u16 i = 0; i < QUEUE_SIZE; i++) {
        u8 *buffer = kmalloc(BUFFER_SIZE);
        if (buffer == NULL)
            return -1;

        struct queue_entry *entry = &daemon_queue.entries[i];
        entry->buffer = buffer;
    }

    daemon_queue.head = daemon_queue.entries;
    daemon_queue.tail = daemon_queue.entries;
    daemon_queue.count = 0;

    if (launch_task(net_daemon_task)) {
        free_net_daemon();
        return -1;
    }

    return 0;
}

void free_net_daemon(void) {
    for (u16 i = 0; i < QUEUE_SIZE; i++)
        kfree(daemon_queue.entries[i].buffer);
}

int net_daemon_add_frame(void *frame, u16 size) {
    // queue is full
    if (daemon_queue.head == daemon_queue.tail && daemon_queue.count != 0)
        return -1;

    struct queue_entry *entry = daemon_queue.tail;
    memcpy(entry->buffer, frame, size);
    entry->size = size;

    if (daemon_queue.tail + 1 >= daemon_queue.entries + QUEUE_SIZE)
        daemon_queue.tail = daemon_queue.entries;
    else daemon_queue.tail++;
    daemon_queue.count++;

    return 0;
}
