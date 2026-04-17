#ifndef ISR_H
#define ISR_H

#include "types.h"
#include "idt.h"

typedef void (*irq_handler_t)(struct interrupt_frame* frame);

void irq_register_handler(uint8_t irq, irq_handler_t handler);
void isr_handler(struct interrupt_frame* frame);
void irq_handler(struct interrupt_frame* frame);

#endif 
