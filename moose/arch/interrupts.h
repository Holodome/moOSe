#pragma once

#include <types.h>

struct registers_state;

struct interrupt_handler {
    void (*handle_interrupt)(const struct registers_state *ctx);

    u8 number;
};

void enable_interrupt(struct interrupt_handler *handler);
void disable_interrupt(struct interrupt_handler *handler);
