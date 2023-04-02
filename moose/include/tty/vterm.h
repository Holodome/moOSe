//
// virtual terminal
//
#pragma once

#include <arch/refcount.h>
#include <types.h>

struct vterm_cell {
    char c;
};

struct vterm {
    refcount_t refcnt;
    struct console *console;

    struct vterm_cell *cells;
};

struct vterm *create_vterm(struct console *console);
void release_vterm(struct vterm *term);

void vterm_putc(struct vterm *term, int c);
void vterm_write(struct vterm *term, const char *str, size_t count);
