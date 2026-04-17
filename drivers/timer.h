#ifndef TIMER_H
#define TIMER_H

#include "stdint.h"


#define PIT_CHANNEL0    0x40
#define PIT_CHANNEL1    0x41
#define PIT_CHANNEL2    0x42
#define PIT_COMMAND     0x43
#define PIT_FREQUENCY   1193182


#define PIC1_COMMAND    0x20
#define PIC1_DATA       0x21
#define PIC2_COMMAND    0xA0
#define PIC2_DATA       0xA1


#define PIC_EOI         0x20  


typedef struct {
    uint32_t ticks;
    uint32_t frequency;
    uint32_t seconds;
    uint32_t minutes;
    uint32_t hours;
} timer_state_t;


void pic_init(void);


void pic_send_eoi(uint8_t irq);


void pic_mask_irq(uint8_t irq);


void pic_unmask_irq(uint8_t irq);



void timer_init(uint32_t frequency);


void timer_handler(void);


uint32_t timer_get_ticks(void);


void timer_reset_ticks(void);


void timer_sleep(uint32_t milliseconds);


void timer_get_state(timer_state_t* state);

#endif 
