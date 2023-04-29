#pragma once

#include <list.h>

struct registers_state;

enum irq_result {
    IRQ_NONE,
    IRQ_HANDLED
};

typedef enum irq_result irqresult_t;

struct interrupt_handler {
    struct list_head list;

    unsigned number;
    const char *name;
    void *dev;
    irqresult_t (*handle_interrupt)(void *dev,
                                    const struct registers_state *ctx);
};

void init_interrupts(void);
void enable_interrupt(struct interrupt_handler *handler);
void disable_interrupt(struct interrupt_handler *handler);

// This is called from within the assembly
void __isr_handler(struct registers_state *regs);
