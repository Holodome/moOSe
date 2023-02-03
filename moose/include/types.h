#pragma once

#include <arch/amd64/types.h>

typedef ssize_t off_t;
#define static_assert(_x) _Static_assert(_x, #_x)
#define ARRAY_SIZE(_arr) (sizeof(_arr) / sizeof(*_arr))
#define offsetof(_struct, _field) ((size_t)(&((_struct *)(0))->_field))
#define container_of(_ptr, _type, _member)                                     \
    ({                                                                         \
        const typeof(((_type *)0)->_member) *__mptr = (_ptr);                  \
        (_type *)((char *)__mptr - offsetof(_type, _member));                  \
    })

struct list_head {
    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(_name)                                                  \
    { &(_name), &(_name) }
#define LiST_HEAD(_name) struct list_head _name = LIST_HEAD_INIT(_name)

static inline void init_list_head(struct list_head *head) {
    head->next = head;
    head->prev = head;
}

static inline void list_add(struct list_head *old, struct list_head *new) {
    old->next->prev = new;
    new->next = old->next;
    new->prev = old;
    old->next = new;
}

static inline void list_add_tail(struct list_head *old, struct list_head *new) {
    old->prev->next = new;
    new->prev = old->prev;
    new->next = old;
    old->prev = new;
}


