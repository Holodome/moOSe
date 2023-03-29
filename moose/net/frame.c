#include <fs/posix.h>
#include <mm/kmalloc.h>
#include <net/frame.h>
#include <net/inet.h>
#include <panic.h>
#include <sched/spinlock.h>
#include <string.h>

#define FREE_FRAMES_COUNT 32

static LIST_HEAD(free_list);

static spinlock_t lock = SPIN_LOCK_INIT();

static struct net_frame *alloc_net_frame(void) {
    struct net_frame *frame = kmalloc(sizeof(*frame) + FRAME_BUFFER_SIZE);
    if (frame == NULL)
        return NULL;

    memset(frame, 0, sizeof(*frame));
    frame->buffer = frame + 1;

    return frame;
}

int init_net_frames(void) {
    for (size_t i = 0; i < FREE_FRAMES_COUNT; i++) {
        struct net_frame *frame = alloc_net_frame();
        if (frame == NULL)
            return -ENOMEM;

        list_add(&frame->list, &free_list);
    }

    return 0;
}

static void init_net_frame(struct net_frame *frame, enum frame_type type) {
    void *buffer = frame->buffer;
    memset(frame->buffer, 0, FRAME_BUFFER_SIZE);
    memset(frame, 0, sizeof(*frame));
    frame->buffer = buffer;

    switch (type) {
    case SEND_FRAME:
        frame->head = frame->buffer + MAX_HEADER_SIZE;
        frame->payload = frame->head;
        break;
    case RECEIVE_FRAME:
        frame->head = frame->buffer;
        break;
    default:
        panic("unsupported frame type\n");
    }
}

struct net_frame *get_free_net_frame(enum frame_type type) {
    u64 flags;
    spin_lock_irqsave(&lock, flags);

    struct net_frame *frame =
        list_next_or_null(&free_list, &free_list, struct net_frame, list);
    if (frame == NULL) {
        spin_unlock_irqrestore(&lock, flags);
        return NULL;
    }

    list_remove(&frame->list);
    init_net_frame(frame, type);

    spin_unlock_irqrestore(&lock, flags);
    return frame;
}

void release_net_frame(struct net_frame *frame) {
    u64 flags;
    spin_lock_irqsave(&lock, flags);
    list_add(&frame->list, &free_list);
    spin_unlock_irqrestore(&lock, flags);
}

void push_net_frame_head(struct net_frame *frame, size_t offset) {
    frame->head += offset;
    frame->size -= offset;
}

void pull_net_frame_head(struct net_frame *frame, size_t offset) {
    frame->head -= offset;
    frame->size += offset;
}

void inc_net_frame_size(struct net_frame *frame, size_t increment) {
    frame->size += increment;
}

size_t get_net_frame_size(struct net_frame *frame) {
    return frame->size;
}
