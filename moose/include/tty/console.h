#pragma once

#include <arch/refcount.h>
#include <sched/locks.h>
#include <types.h>

struct console;

enum console_color {
    CONSOLE_BLACK,
    CONSOLE_BLUE,
    CONSOLE_GREEN,
    CONSOLE_CYAN,
    CONSOLE_RED,
    CONSOLE_MAGENTA,
    CONSOLE_BROWN,
    CONSOLE_LIGHT_GRAY,
    CONSOLE_DARK_GRAY,
    CONSOLE_BRIGHT_BLUE,
    CONSOLE_BRIGHT_GREEN,
    CONSOLE_BRIGHT_CYAN,
    CONSOLE_BRIGHT_RED,
    CONSOLE_BRIGHT_MAGENTA,
    CONSOLE_YELLOW,
    CONSOLE_WHITE
};

struct console_ops {
    void (*release)(struct console *console);
    void (*clear)(struct console *console, size_t x, size_t y, size_t length);
    void (*write)(struct console *console, size_t x, size_t y, int c,
                     enum console_color bg, enum console_color fg);
    void (*flush)(struct console *console, size_t x, size_t y, size_t width,
                  size_t height);
    void (*set_cursor)(struct console *console, size_t x, size_t y);
};

struct console {
    refcount_t refcnt;
    size_t width, height;
    size_t x, y;
    enum console_color default_bg, default_fg;

    void *private;
    const struct console_ops *ops;
};

struct console *create_empty_console(void);
void console_release(struct console *console);
void console_clear(struct console *console, size_t x, size_t y, size_t length);
void console_write2(struct console *console, size_t x, size_t y, int c,
                    enum console_color bg, enum console_color fg);
void console_write1(struct console *console, size_t x, size_t y, int c);
void console_write(struct console *console, int c);
void console_flush(struct console *console, size_t x, size_t y, size_t width,
         size_t height);
