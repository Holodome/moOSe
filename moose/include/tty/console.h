#pragma once

#include <arch/refcount.h>
#include <sched/locks.h>
#include <types.h>

struct console;

// NOTE: These are designed to map 1 to 1 to VGA colors
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
    void (*set_cursor)(struct console *console, size_t x, size_t y);
    void (*show_cursor)(struct console *console, size_t x, size_t y);
    void (*hide_cursor)(struct console *console);
};

struct console {
    refcount_t refcnt;
    size_t width, height;

    void *private;
    const struct console_ops *ops;
};

struct console *create_empty_console(void);
void console_release(struct console *console);
void console_clear(struct console *console, size_t x, size_t y, size_t length);
void console_clear_all(struct console *console);
void console_write(struct console *console, size_t x, size_t y, int c,
                   enum console_color bg, enum console_color fg);
void console_set_cursor(struct console *console, size_t x, size_t y);
void console_hide_cursor(struct console *console);
void console_show_cursor(struct console *console);
