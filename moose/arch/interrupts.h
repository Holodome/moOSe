#pragma once

#include <types.h>

struct isr_context;

struct interrupt_handler {
    void (*handle_interrupt)(const struct isr_context *ctx);

    u8 number;
};

void enable_interrupt(struct interrupt_handler *handler);
void disable_interrupt(struct interrupt_handler *handler);
