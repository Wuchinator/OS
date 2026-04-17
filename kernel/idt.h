#ifndef IDT_H
#define IDT_H

#include "types.h"


#define IDT_GATE_TASK   0x05
#define IDT_GATE_INT16  0x06
#define IDT_GATE_TRAP16 0x07
#define IDT_GATE_INT32  0x0E  
#define IDT_GATE_TRAP32 0x0F  


#define IDT_FLAG_PRESENT    0x80
#define IDT_FLAG_RING0      0x00
#define IDT_FLAG_RING3      0x60
#define IDT_FLAG_RING1      0x20
#define IDT_FLAG_RING2      0x40


struct idt_entry {
    uint16_t base_low;      
    uint16_t sel;           
    uint8_t  always0;       
    uint8_t  flags;         
    uint16_t base_high;     
} __attribute__((packed));


struct idt_ptr {
    uint16_t limit;         
    uint32_t base;          
} __attribute__((packed));


struct interrupt_frame {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_num, err_code;
    uint32_t eip, cs, eflags, user_esp, user_ss;
} __attribute__((packed));


void idt_init(void);


void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

#endif 