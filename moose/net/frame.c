#include <assert.h>
#include <errno.h>
#include <mm/kmalloc.h>
#include <net/frame.h>
#include <net/inet.h>
#include <sched/locks.h>
#include <string.h>

#define FREE_FRAMES_COUNT 32

static LIST_HEAD(free_list);

static spinlock_t lock = INIT_SPIN_LOCK();

static struct net_frame *alloc_net_frame(void) {
    struct net_frame *frame = kmalloc(sizeof(*frame) + FRAME_BUFFER_SIZE);
    if (frame == NULL)
        return NULL;

    frame->buffer = frame + 1;

    return frame;
}

static void free_net_frame(struct net_frame *frame) {
    kfree(frame);
}

int init_net_frames(void) {
    for (size_t i = 0; i < FREE_FRAMES_COUNT; i++) {
        struct net_frame *frame = alloc_net_frame();
        if (frame == NULL) {
            destroy_net_frames();
            return -ENOMEM;
        }

        list_add(&frame->list, &free_list);
    }

    return 0;
}

void destroy_net_frames(void) {
    struct net_frame *frame;
    struct net_frame *temp;
    list_for_each_entry_safe(frame, temp, &free_list, list) {
        list_remove(&frame->list);
        free_net_frame(frame);
    }
}

static struct net_frame *get_empty_net_frame(void) {
    cpuflags_t flags = spin_lock_irqsave(&lock);

    struct net_frame *frame =
        list_first_or_null(&free_list, struct net_frame, list);
    if (frame == NULL) {
        spin_unlock_irqrestore(&lock, flags);
        return NULL;
    }

    list_remove(&frame->list);
    spin_unlock_irqrestore(&lock, flags);

    void *buffer = frame->buffer;
    memset(frame->buffer, 0, FRAME_BUFFER_SIZE);
    memset(frame, 0, sizeof(*frame));
    frame->buffer = buffer;

    return frame;
}

struct net_frame *get_empty_send_net_frame(void) {
    struct net_frame *frame = get_empty_net_frame();
    if (frame == NULL)
        return NULL;

    frame->head = frame->buffer + MAX_HEADER_SIZE;
    frame->payload = frame->head;

    return frame;
}

struct net_frame *get_empty_receive_net_frame(void) {
    struct net_frame *frame = get_empty_net_frame();
    if (frame == NULL)
        return NULL;

    frame->head = frame->buffer;

    return frame;
}

void release_net_frame(struct net_frame *frame) {
    cpuflags_t flags = spin_lock_irqsave(&lock);
    list_add(&frame->list, &free_list);
    spin_unlock_irqrestore(&lock, flags);
}

void push_net_frame_head(struct net_frame *frame, size_t offset) {
    expects(frame->size >= offset);
    frame->head += offset;
    frame->size -= offset;
}

void pull_net_frame_head(struct net_frame *frame, size_t offset) {
    expects(frame->head - offset >= frame->buffer);
    frame->head -= offset;
    frame->size += offset;
}

void inc_net_frame_size(struct net_frame *frame, size_t increment) {
    frame->size += increment;
}
