#include <rbtree.h>

static struct rb_node *rb_parent(const struct rb_node *node) {
    return (void *)(node->parent__color & ~1);
}

#define rb_is_black(_node) (!rb_color(_node))
#define rb_is_red(_node) (rb_color(_node))
static int rb_color(const struct rb_node *node) {
    return node->parent__color & 1;
}

static void rb_set_parent(struct rb_node *node, struct rb_node *p) {
    node->parent__color = (node->parent__color & 1) | (uintptr_t)p;
}

static void rb_set_red(struct rb_node *node) {
    node->parent__color &= ~1;
}
static void rb_set_black(struct rb_node *node) {
    node->parent__color |= 1;
}

static void rb_set_color(struct rb_node *node, int color) {
    node->parent__color = (node->parent__color & ~1) | color;
}

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

static void rb_rotate_left(struct rb_node *node, struct rb_node **root) {
    struct rb_node *right = node->right;
    struct rb_node *parent = rb_parent(node);

    if ((node->right = right->left))
        rb_set_parent(right->left, node);
    right->left = node;

    rb_set_parent(right, parent);

    if (parent) {
        if (node == parent->left)
            parent->left = right;
        else
            parent->right = right;
    } else {
        *root = right;
    }
    rb_set_parent(node, right);
}

static void rb_rotate_right(struct rb_node *node, struct rb_node **root) {
    struct rb_node *left = node->left;
    struct rb_node *parent = rb_parent(node);

    if ((node->left = left->right))
        rb_set_parent(left->right, node);
    left->right = node;

    rb_set_parent(left, parent);

    if (parent) {
        if (node == parent->right)
            parent->right = left;
        else
            parent->left = left;
    } else {
        *root = left;
    }
    rb_set_parent(node, left);
}

void rb_insert_color(struct rb_node *node, struct rb_node **root) {
    struct rb_node *parent, *gparent;

    while ((parent = rb_parent(node)) && rb_is_red(parent)) {
        gparent = rb_parent(parent);

        if (parent == gparent->left) {
            {
                struct rb_node *uncle = gparent->right;
                if (uncle && rb_is_red(uncle)) {
                    rb_set_black(uncle);
                    rb_set_black(parent);
                    rb_set_red(gparent);
                    node = gparent;
                    continue;
                }
            }

            if (parent->right == node) {
                struct rb_node *tmp;
                rb_rotate_left(parent, root);
                tmp = parent;
                parent = node;
                node = tmp;
            }

            rb_set_black(parent);
            rb_set_red(gparent);
            rb_rotate_right(gparent, root);
        } else {
            {
                struct rb_node *uncle = gparent->left;
                if (uncle && rb_is_red(uncle)) {
                    rb_set_black(uncle);
                    rb_set_black(parent);
                    rb_set_red(gparent);
                    node = gparent;
                    continue;
                }
            }

            if (parent->left == node) {
                struct rb_node *tmp;
                rb_rotate_right(parent, root);
                tmp = parent;
                parent = node;
                node = tmp;
            }

            rb_set_black(parent);
            rb_set_red(gparent);
            rb_rotate_left(gparent, root);
        }
    }

    rb_set_black(*root);
}

void rb_erase_color(struct rb_node *node, struct rb_node *parent,
                    struct rb_node **root) {
    struct rb_node *other;

    while ((!node || rb_is_black(node)) && node != *root) {
        if (parent->left == node) {
            other = parent->right;
            if (rb_is_red(other)) {
                rb_set_black(other);
                rb_set_red(parent);
                rb_rotate_left(parent, root);
                other = parent->right;
            }
            if ((!other->left || rb_is_black(other->left)) &&
                (!other->right || rb_is_black(other->right))) {
                rb_set_red(other);
                node = parent;
                parent = rb_parent(node);
            } else {
                if (!other->right || rb_is_black(other->right)) {
                    rb_set_black(other->left);
                    rb_set_red(other);
                    rb_rotate_right(other, root);
                    other = parent->right;
                }
                rb_set_color(other, rb_color(parent));
                rb_set_black(parent);
                rb_set_black(other->right);
                rb_rotate_left(parent, root);
                node = *root;
                break;
            }
        } else {
            other = parent->left;
            if (rb_is_red(other)) {
                rb_set_black(other);
                rb_set_red(parent);
                rb_rotate_right(parent, root);
                other = parent->left;
            }
            if ((!other->left || rb_is_black(other->left)) &&
                (!other->right || rb_is_black(other->right))) {
                rb_set_red(other);
                node = parent;
                parent = rb_parent(node);
            } else {
                if (!other->left || rb_is_black(other->left)) {
                    rb_set_black(other->right);
                    rb_set_red(other);
                    rb_rotate_left(other, root);
                    other = parent->left;
                }
                rb_set_color(other, rb_color(parent));
                rb_set_black(parent);
                rb_set_black(other->left);
                rb_rotate_right(parent, root);
                node = *root;
                break;
            }
        }
    }
    if (node)
        rb_set_black(node);
}

void rb_erase(struct rb_node *node, struct rb_node **root) {
    struct rb_node *child, *parent;
    int color;

    if (!node->left)
        child = node->right;
    else if (!node->right)
        child = node->left;
    else {
        struct rb_node *old = node, *left;

        node = node->right;
        while ((left = node->left) != NULL)
            node = left;

        if (rb_parent(old)) {
            if (rb_parent(old)->left == old)
                rb_parent(old)->left = node;
            else
                rb_parent(old)->right = node;
        } else {
            *root = node;
        }

        child = node->right;
        parent = rb_parent(node);
        color = rb_color(node);

        if (parent == old) {
            parent = node;
        } else {
            if (child)
                rb_set_parent(child, parent);
            parent->left = child;

            node->right = old->right;
            rb_set_parent(old->right, node);
        }

        node->parent__color = old->parent__color;
        node->left = old->left;
        rb_set_parent(old->left, node);

        goto color;
    }

    parent = rb_parent(node);
    color = rb_color(node);

    if (child)
        rb_set_parent(child, parent);
    if (parent) {
        if (parent->left == node)
            parent->left = child;
        else
            parent->right = child;
    } else {
        *root = child;
    }

color:
    if (color == RB_BLACK)
        rb_erase_color(child, parent, root);
}
