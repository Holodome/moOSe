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

