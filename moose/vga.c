#include "vga.h"

#include "arch/ports.h"

#define VMEM_ADDR 0xb8000
#define VMEM ((volatile u8 *)VMEM_ADDR)

#define MAX_ROWS 25
#define MAX_COLS 80
#define MAX_CURSOR MAX_ROWS *MAX_COLS

#define WHITE_ON_BLACK 0x0f

#define P_SCRCTL 0x3d4
#define P_SCRDAT 0x3d5

static u16 get_cursor(void) {
    port_u8_out(P_SCRCTL, 14);
    u16 offset = port_u8_in(P_SCRDAT) << 8;
    port_u8_out(P_SCRCTL, 15);
    offset |= port_u8_in(P_SCRDAT);
    return offset;
}

static void set_cursor(u16 cursor) {
    port_u8_out(P_SCRCTL, 14);
    port_u8_out(P_SCRDAT, cursor >> 8);
    port_u8_out(P_SCRCTL, 15);
    port_u8_out(P_SCRDAT, cursor & 0xff);
}

static void setc(u16 location, u8 cp, u8 flags) {
    volatile u8 *slot = VMEM + location * 2;
    *slot++ = cp;
    *slot = flags;
}

static void cursor_to_pos(u16 cursor, u16 *col, u16 *row) {
    *row = cursor / MAX_COLS;
    *col = cursor % MAX_COLS;
}

static u16 pos_to_cursor(u16 col, u16 row) { return (row * MAX_COLS + col); }

static u16 adjust_cursor(u16 cursor) {
    if (cursor >= MAX_CURSOR)
        cursor -= MAX_CURSOR;
    return cursor;
}

static u16 next_line(u16 cursor) {
    u16 col, row;
    cursor_to_pos(cursor, &col, &row);
    col = 0;
    row += 1;
    cursor = adjust_cursor(pos_to_cursor(col, row));
    return cursor;
}

void kputc(int cp) {
    u16 cursor = get_cursor();
    if (cp == '\n')
        cursor = next_line(cursor);
    else
        setc(cursor++, cp, WHITE_ON_BLACK);
    set_cursor(cursor);
}

void kputs(const char *str) {
    for (;;) {
        int c = *str++;
        if (c == '\0')
            break;

        kputc(c);
    }
}

void kcls(void) {
    for (u16 i = 0; i < MAX_ROWS * MAX_COLS; ++i)
        setc(i, ' ', WHITE_ON_BLACK);

    set_cursor(0);
}
