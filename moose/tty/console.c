#include <mm/kmalloc.h>
#include <tty/console.h>

struct console *create_empty_console(void) {
    struct console *console = kzalloc(sizeof(*console));
    if (!console)
        return NULL;

    refcount_set(&console->refcnt, 1);

    return console;
}

void console_release(struct console *console) {
    if (refcount_dec_and_test(&console->refcnt)) {
        console->ops->release(console);
        kfree(console);
    }
}

void console_clear(struct console *console, size_t x, size_t y, size_t length) {
    console->ops->clear(console, x, y, length);
}

void console_write(struct console *console, size_t x, size_t y, int c,
                   enum console_color bg, enum console_color fg) {
    console->ops->write(console, x, y, c, bg, fg);
}

void console_clear_all(struct console *console) {
    console_clear(console, 0, 0, console->width * console->height);
}

void console_set_cursor(struct console *console, size_t x, size_t y) {
    console->ops->set_cursor(console, x, y);
}

void console_hide_cursor(struct console *console) {
    console->ops->hide_cursor(console);
}

void console_show_cursor(struct console *console, size_t x, size_t y) {
    console->ops->show_cursor(console, x, y);
}
