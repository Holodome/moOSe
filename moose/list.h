#pragma once

#include <moose/types.h>

struct list_head {
    struct list_head *next, *prev;
};

#define INIT_LIST_HEAD(_name)                                                  \
    { &(_name), &(_name) }
#define LIST_HEAD(_name) struct list_head _name = INIT_LIST_HEAD(_name)

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

static __forceinline __nodiscard int list_is_empty(const struct list_head *it) {
    return it->next == it->prev;
}

#define list_first_entry(_item, _type, _member)                                \
    list_entry((_item)->next, _type, _member)

#define list_next_entry(_item, _member)                                        \
    list_entry((_item)->_member.next, typeof(*(_item)), _member)

#define list_entry_is_head(_item, _head, _member) (&(_item)->_member == (_head))

#define list_for_each(_iter, _head)                                            \
    for (struct list_head *_iter = (_head)->next; _iter != _head;              \
         _iter = _iter->next)

#define list_for_each_safe(_iter, _temp, _head)                                \
    for (struct list_head *_iter = (_head)->next, _temp = _iter->next;         \
         _iter != _head; _iter = _temp, _temp = (_iter)->next)

#define list_entry(_ptr, _type, _member) container_of(_ptr, _type, _member)

#define list_for_each_entry(_iter, _head, _member)                             \
    for ((_iter) = list_first_entry(_head, typeof(*(_iter)), _member);         \
         !list_entry_is_head(_iter, _head, _member);                           \
         (_iter) = list_next_entry(_iter, _member))

#define list_for_each_entry_safe(_iter, _temp, _head, _member)                 \
    for (_iter = list_first_entry(_head, typeof(*_iter), _member),             \
        _temp = list_next_entry(_iter, _member);                               \
         !list_entry_is_head(_iter, _head, _member);                           \
         _iter = _temp, _temp = list_next_entry(_iter, _member))

#define list_prev_or_null(_ptr, _head, _type, _member)                         \
    ({                                                                         \
        struct list_head *__prev = (_ptr)->prev;                               \
        __prev != (_head) ? list_entry(__prev, _type, _member) : NULL;         \
    })

#define list_prev_entry_or_null(_ptr, _head, _member)                          \
    list_prev_or_null(&(_ptr)->_member, _head, typeof(*(_ptr)), _member)

#define list_last_or_null(_head, _type, _member)                               \
    list_prev_or_null(_head, _head, _type, _member)

#define list_next_or_null(_ptr, _head, _type, _member)                         \
    ({                                                                         \
        struct list_head *__next = (_ptr)->next;                               \
        __next != (_head) ? list_entry(__next, _type, _member) : NULL;         \
    })

#define list_next_entry_or_null(_ptr, _head, _member)                          \
    list_next_or_null(&(_ptr)->_member, _head, typeof(*(_ptr)), _member)

#define list_first_or_null(_head, _type, _member)                              \
    list_next_or_null(_head, _head, _type, _member)
