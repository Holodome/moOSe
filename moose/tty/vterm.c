#include <moose/ctype.h>
#include <moose/mm/kmalloc.h>
#include <moose/string.h>
#include <moose/tty/console.h>
#include <moose/tty/vterm.h>

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
    term->lines = kzalloc(sizeof(*term->lines) * console->height * 2);
    if (!term->lines) {
        kfree(term->cells);
        kfree(term);
        return NULL;
    }
    init_spin_lock(&term->lock);
    term->console = console;
    term->default_bg = CONSOLE_BLACK;
    term->default_fg = CONSOLE_WHITE;
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

static struct vterm_line *vterm_get_line(struct vterm *term, long n) {
    return term->lines + term->console->height + n;
}

static struct vterm_cell *vterm_get_cell(struct vterm *term, long x, long y) {
    return term->cells + term->console->height * term->console->width +
           (y * term->console->width) + x;
}

static void vterm_putc_at(struct vterm *term, int c, size_t x, size_t y) {
    struct vterm_line *line = vterm_get_line(term, y);
    struct vterm_cell *cell = vterm_get_cell(term, x, y);

    cell->bg = term->default_bg;
    cell->fg = term->default_fg;
    cell->c = c;
    line->is_dirty = 1;
}

static void vterm_flush(struct vterm *term) {
    for (size_t y = 0; y < term->console->height; ++y) {
        struct vterm_line *line = vterm_get_line(term, y);
        if (!line->is_dirty)
            continue;

        for (size_t x = 0; x < term->console->width; ++x) {
            const struct vterm_cell *cell = vterm_get_cell(term, x, y);
            console_write(term->console, x, y, cell->c, cell->bg, cell->fg);
        }
        line->is_dirty = 0;
    }

    console_set_cursor(term->console, term->x, term->y);
}

static void __vterm_move_up(struct vterm *term, size_t count) {
    size_t w = term->console->width;
    size_t h = term->console->height;
    memmove(term->lines, term->lines + count,
            (h * 2 - count) * sizeof(*term->lines));
    memmove(term->cells, term->cells + count * w,
            sizeof(*term->cells) * w * (h * 2 - count));
    for (size_t row = 0; row < h * 2; ++row)
        term->lines[row].is_dirty = 1;

    for (size_t row = 2 * h - count; row < 2 * h; ++row) {
        for (size_t col = 0; col < w; ++col) {
            struct vterm_cell *cell = term->cells + row * w + col;
            cell->c = ' ';
            cell->bg = term->default_bg;
            cell->fg = term->default_fg;
        }
    }
}

static void vterm_newline(struct vterm *term) {
    ++term->y;
    if (term->y >= term->console->height) {
        __vterm_move_up(term, 1);
        --term->y;
    }
}

void vterm_write(struct vterm *term, const char *str, size_t count) {
    cpuflags_t flags = spin_lock_irqsave(&term->lock);

    for (size_t i = 0; i < count; ++i) {
        int c = str[i];
        if (c == '\n') {
            term->x = 0;
            vterm_newline(term);
        } else {
            vterm_putc_at(term, c, term->x, term->y);
            ++term->x;
            if (term->x >= term->console->width) {
                term->x = 0;
                vterm_newline(term);
            }
        }
    }

    vterm_flush(term);
    spin_unlock_irqrestore(&term->lock, flags);
}
