#include <ctype.h>
#include <mm/kmalloc.h>
#include <tty/console.h>
#include <tty/vterm.h>

struct vterm *create_vterm(struct console *console) {
    struct vterm *term = kzalloc(sizeof(*term));
    if (!term)
        return NULL;

    // make history be twice of screen size
    size_t cell_count = console->width * console->height * 2;
    term->cells = kzalloc(sizeof(*term->cells) * cell_count);
    if (!term->cells) {
        kfree(term);
        return NULL;
    }
    term->lines = kzalloc(sizeof(*term->lines) * console->height);
    if (!term->lines) {
        kfree(term->cells);
        kfree(term);
        return NULL;
    }
    spin_lock_init(&term->lock);
    term->console = console;
    refcount_set(&term->refcnt, 1);

    console_clear_all(console);

    return term;
}

void release_vterm(struct vterm *term) {
    if (refcount_dec_and_test(&term->refcnt)) {
        console_release(term->console);
        kfree(term->cells);
        kfree(term->lines);
        kfree(term);
    }
}

static void vterm_putc_at(struct vterm *term, int c, size_t x, size_t y) {
    struct vterm_line *line = term->lines + y;
    struct vterm_cell *cell = term->cells + y * term->console->width + x;

    if (!isascii(c))
        c = ' ';

    cell->bg = CONSOLE_BLACK;
    cell->fg = CONSOLE_WHITE;
    cell->c = c;
    line->is_dirty = 1;
    if (!iscntrl(c) && line->length < x)
        line->length = x + 1;
}

static void vterm_flush(struct vterm *term) {
    for (size_t y = 0; y < term->console->height; ++y) {
        struct vterm_line *line = term->lines + y;
        if (!line->is_dirty)
            continue;

        for (size_t x = 0; x < line->length; ++x) {
            struct vterm_cell *cell =
                term->cells + y * term->console->width + x;
            console_write(term->console, term->x, term->y, cell->c, cell->bg,
                          cell->fg);
        }
        line->is_dirty = 0;
    }

    console_set_cursor(term->console, term->x, term->y);
}

static void vterm_scroll_down(struct vterm *term, size_t count) {
    //
}

void vterm_write(struct vterm *term, const char *str, size_t count) {
    cpuflags_t flags;
    spin_lock_irqsave(&term->lock, flags);

    for (size_t i = 0; i < count; ++i) {
        int c = str[i];
        if (c == '\n') {
            ++term->y;
            if (term->y >= term->console->height) {
                vterm_scroll_down(term, 1);
                --term->y;
            }
        } else {
            vterm_putc_at(term, c, term->x, term->y);
        }
    }

    vterm_flush(term);
    spin_unlock_irqrestore(&term->lock, flags);
}
