#pragma once

#include <types.h>

#define RB_BLACK 0
#define RB_RED 1

struct rb_node {
    // store pointer to parent or'ed with color
    // we can do that because parent structure is aligned by 8 (as struct
    // rb_node is aligned that way)
    uintptr_t parent__color;
    struct rb_node *left;
    struct rb_node *right;
};

#define rb_entry(_ptr, _type, _member) container_of(_ptr, _type, _member)
#define rb_entry_safe(_ptr, _type, _member)                                    \
    ({                                                                         \
        typeof(_ptr) __ptr = (_ptr);                                           \
        __ptr ? rb_entry(__ptr, _type, _member) : NULL;                        \
    })

struct rb_node *rb_next(const struct rb_node *node);
struct rb_node *rb_prev(const struct rb_node *node);
struct rb_node *rb_first(const struct rb_node *root);
struct rb_node *rb_last(const struct rb_node *root);

static __forceinline void rb_link_node(struct rb_node *node,
                                       struct rb_node *parent) {
    node->parent__color = (uintptr_t)parent;
    node->left = node->right = NULL;
}

void rb_insert_color(struct rb_node *node, struct rb_node **root);
void rb_erase(struct rb_node *node, struct rb_node **root);
