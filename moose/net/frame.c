#include <net/frame.h>
#include <mm/kmalloc.h>
#include <net/inet.h>
#include <sched/spinlock.h>
#include <errno.h>

#define FRAMES_COUNT 20

LIST_HEAD(free_list);

static spinlock_t lock = SPIN_LOCK_INIT();

int init_net_frames(void) {
    for (size_t i = 0; i < FRAMES_COUNT; i++) {
        struct net_frame *frame = kmalloc(sizeof(*frame) + ETH_FRAME_MAX_SIZE);
        if (frame == NULL)
            return -ENOMEM;

        frame->data = frame + 1;
        frame->size = ETH_FRAME_MAX_SIZE;
        list_add(&frame->list, &free_list);
    }

    return 0;
}

struct net_frame *alloc_net_frame(void) {
    spin_lock(&lock);

    struct net_frame *frame =
        list_next_or_null(&free_list, &free_list, struct net_frame, list);
    if (frame == NULL)
        return NULL;

    list_remove(&frame->list);

    spin_unlock(&lock);
    return frame;
}

void free_net_frame(struct net_frame *frame) {
    spin_lock(&lock);
    list_add(&frame->list, &free_list);
    spin_unlock(&lock);
}
