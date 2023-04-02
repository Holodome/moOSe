//
// virtual terminal
//
#pragma once

#include <arch/refcount.h>
#include <sched/locks.h>
#include <tty/console.h>

struct vterm_cell {
    char c;
    enum console_color bg;
    enum console_color fg;
};

struct vterm_line {
    int is_dirty;
    /* int length; */
};

struct vterm {
    refcount_t refcnt;
    struct console *console;
    size_t x, y;
    enum console_color default_bg, default_fg;

    struct vterm_cell *cells;
    struct vterm_line *lines;
    spinlock_t lock;
};

struct vterm *create_vterm(struct console *console);
void release_vterm(struct vterm *term);

void vterm_write(struct vterm *term, const char *str, size_t count);
void vterm_move_up(struct vterm *term, size_t count);
void vterm_move_down(struct vterm *term, size_t count);
