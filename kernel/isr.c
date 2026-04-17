#include "isr.h"
#include "idt.h"
#include "../drivers/vga.h"


static irq_handler_t irq_handlers[16] = {0};


void irq_register_handler(uint8_t irq, irq_handler_t handler) {
    if (irq < 16) {
        irq_handlers[irq] = handler;
    }
}


static const char* exception_names[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 FPU Error",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception"
};


void isr_handler(struct interrupt_frame* frame) {
    if (frame->int_num < 21) {
        vga_print_string("EXCEPTION: ", 0, 22, 0x04);
        vga_print_string(exception_names[frame->int_num], 11, 22, 0x04);
    }
    
    __asm__ volatile("cli; hlt");
}


void irq_handler(struct interrupt_frame* frame) {
    
    uint8_t irq = (uint8_t)(frame->int_num - 32);

    if (irq < 16 && irq_handlers[irq]) {
        irq_handlers[irq](frame);
    }
}
