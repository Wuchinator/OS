#include "timer.h"
#include "string.h"
#include "io.h"


static volatile timer_state_t timer_state = {0};


void pic_init(void) {
    
    uint8_t a1 = inb(PIC1_DATA);
    uint8_t a2 = inb(PIC2_DATA);
    
    
    outb(PIC1_COMMAND, 0x11);
    __asm__ volatile("jmp 1f\n1:\n");  
    outb(PIC2_COMMAND, 0x11);
    __asm__ volatile("jmp 1f\n1:\n");
    
    
    outb(PIC1_DATA, 0x20);  
    __asm__ volatile("jmp 1f\n1:\n");
    outb(PIC2_DATA, 0x28);  
    __asm__ volatile("jmp 1f\n1:\n");
    
    
    outb(PIC1_DATA, 0x04);  
    __asm__ volatile("jmp 1f\n1:\n");
    outb(PIC2_DATA, 0x02);  
    __asm__ volatile("jmp 1f\n1:\n");
    
    
    outb(PIC1_DATA, 0x01);  
    __asm__ volatile("jmp 1f\n1:\n");
    outb(PIC2_DATA, 0x01);
    __asm__ volatile("jmp 1f\n1:\n");
    
    
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}


void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}


void pic_mask_irq(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    value = inb(port) | (1 << irq);
    outb(port, value);
}


void pic_unmask_irq(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    value = inb(port) & ~(1 << irq);
    outb(port, value);
}


void timer_init(uint32_t frequency) {
    
    timer_state.frequency = frequency;
    timer_state.ticks = 0;
    timer_state.seconds = 0;
    timer_state.minutes = 0;
    timer_state.hours = 0;
    
    
    uint32_t divisor = PIT_FREQUENCY / frequency;
    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);
    
    
    outb(PIT_COMMAND, 0x36);  
    
    
    outb(PIT_CHANNEL0, low);
    outb(PIT_CHANNEL0, high);
}


void timer_handler(void) {
    timer_state.ticks++;

    
    uint32_t ms_per_tick = 1000 / timer_state.frequency;
    uint32_t total_ms = timer_state.ticks * ms_per_tick;

    timer_state.seconds = total_ms / 1000;
    timer_state.minutes = timer_state.seconds / 60;
    timer_state.hours = timer_state.minutes / 60;
    
}


uint32_t timer_get_ticks(void) {
    return timer_state.ticks;
}


void timer_reset_ticks(void) {
    timer_state.ticks = 0;
    timer_state.seconds = 0;
    timer_state.minutes = 0;
    timer_state.hours = 0;
}


void timer_sleep(uint32_t milliseconds) {
    uint32_t target_ticks = (milliseconds * timer_state.frequency) / 1000;
    uint32_t start_ticks = timer_state.ticks;
    
    while ((timer_state.ticks - start_ticks) < target_ticks) {
        
        __asm__ volatile("hlt");
    }
}


void timer_get_state(timer_state_t* state) {
    if (state) {
        *state = timer_state;
    }
}
