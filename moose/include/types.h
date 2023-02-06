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
#define LIST_HEAD(_name) struct list_head _name = LIST_HEAD_INIT(_name)

static inline void init_list_head(struct list_head *head) {
    head->next = head;
    head->prev = head;
}

static inline void __list_add(struct list_head *new, struct list_head *prev,
                              struct list_head *next) {
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

static inline void list_add(struct list_head *new, struct list_head *head) {
    __list_add(new, head, head->next);
}

static inline void list_add_tail(struct list_head *new,
                                 struct list_head *head) {
    __list_add(new, head->prev, head);
}

static inline void list_remove(struct list_head *it) {
    struct list_head *next = it->next;
    struct list_head *prev = it->prev;
    prev->next = next;
    next->prev = prev;
}

#define list_for_each(_iter, _head)                                            \
    for (struct list_head *_iter = (_head)->next; _iter != _head;              \
         _iter = _iter->next)

#define list_entry(_ptr, _type, _member) container_of(_ptr, _type, _member)

#define list_prev_or_null(_ptr, _head, _type, _member)                         \
    ({                                                                         \
        struct list_head *__prev = (_ptr)->prev;                               \
        __prev != (_head) ? list_entry(__prev, _type, _member) : NULL;         \
    })

#define list_next_or_null(_ptr, _head, _type, _member)                         \
    ({                                                                         \
        struct list_head *__next = (_ptr)->next;                               \
        __next != (_head) ? list_entry(__next, _type, _member) : NULL;         \
    })
