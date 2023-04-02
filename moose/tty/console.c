#include <mm/kmalloc.h>
#include <tty/console.h>

struct console *create_empty_console(void) {
    struct console *console = kzalloc(sizeof(*console));
    if (!console)
        return NULL;

    console->default_bg = CONSOLE_BLACK;
    console->default_fg = CONSOLE_WHITE;
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

void console_write2(struct console *console, size_t x, size_t y, int c,
                    enum console_color bg, enum console_color fg) {
    console->ops->write(console, x, y, c, bg, fg);
}

void console_write1(struct console *console, size_t x, size_t y, int c) {
    console_write2(console, x, y, c, console->default_bg, console->default_fg);
}

void console_write(struct console *console, int c) {
    console_write1(console, console->x, console->y, c);
}

void console_flush(struct console *console, size_t x, size_t y, size_t width,
                   size_t height) {
    console->ops->flush(console, x, y, width, height);
}
