#include <rbtree.h>

struct rb_node *rb_next(const struct rb_node *node) {
    if (node->right) {
        node = node->right;
        while (node->left)
            node = node->left;

        return (struct rb_node *)node;
    }

    struct rb_node *parent;
    while ((parent = rb_parent(node)) && node == parent->right)
        node = parent;

    return parent;
}

struct rb_node *rb_prev(const struct rb_node *node) {
    if (node->left) {
        node = node->left;
        while (node->right)
            node = node->right;

        return (struct rb_node *)node;
    }

    struct rb_node *parent;
    while ((parent = rb_parent(node)) && node == parent->left)
        node = parent;

    return parent;
}

struct rb_node *rb_first(const struct rb_node *root) {
    struct rb_node *first = (struct rb_node *)root;
    while (first->left)
        first = first->left;

    return first;
}

struct rb_node *rb_last(const struct rb_node *root) {
    struct rb_node *last = (struct rb_node *)root;
    while (last->right)
        last = last->right;

    return last;
}
