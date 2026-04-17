#include "serial.h"
#include "string.h"

static uint32_t serial_initialized = 0;


static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}


int serial_init(uint16_t port, int baud_rate) {
    
    outb(port + 1, 0x00);

    
    outb(port + 3, 0x80);
    
    
    uint16_t divisor = 115200 / baud_rate;
    outb(port + 0, (divisor >> 0) & 0xFF);  
    outb(port + 1, (divisor >> 8) & 0xFF);  
    
    
    outb(port + 3, 0x03);
    
    
    outb(port + 2, 0xC7);
    
    
    outb(port + 4, 0x0B);
    
    
    serial_initialized |= (1 << (port - COM1));
    
    return 0;
}

int serial_is_initialized(uint16_t port) {
    return (serial_initialized & (1 << (port - COM1))) != 0;
}


int serial_data_available(uint16_t port) {
    return inb(port + 5) & 0x01;
}


static int serial_transmit_empty(uint16_t port) {
    return inb(port + 5) & 0x20;
}


int serial_read(uint16_t port) {
    if (!serial_is_initialized(port)) return -1;
    
    if (serial_data_available(port)) {
        return inb(port);
    }
    
    return -1;
}


void serial_write(uint16_t port, uint8_t data) {
    if (!serial_is_initialized(port)) return;
    
    while (!serial_transmit_empty(port)) {
        __asm__ volatile("pause");
    }
    
    outb(port, data);
}


void serial_write_string(uint16_t port, const char* str) {
    if (!serial_is_initialized(port)) return;
    
    while (*str) {
        serial_write(port, *str);
        str++;
    }
}


void serial_write_hex(uint16_t port, uint32_t value) {
    if (!serial_is_initialized(port)) return;
    
    serial_write_string(port, "0x");
    
    for (int i = 7; i >= 0; i--) {
        uint8_t nibble = (value >> (i * 4)) & 0xF;
        if (nibble < 10) {
            serial_write(port, nibble + '0');
        } else {
            serial_write(port, nibble - 10 + 'A');
        }
    }
}


void serial_write_dec(uint16_t port, int32_t value) {
    if (!serial_is_initialized(port)) return;
    
    if (value < 0) {
        serial_write(port, '-');
        value = -value;
    }
    
    char buffer[12];
    int i = 0;
    
    if (value == 0) {
        serial_write(port, '0');
        return;
    }
    
    while (value > 0 && i < 11) {
        buffer[i++] = (value % 10) + '0';
        value /= 10;
    }
    
    
    while (--i >= 0) {
        serial_write(port, buffer[i]);
    }
}


void serial_enable_interrupts(uint16_t port) {
    if (!serial_is_initialized(port)) return;
    
    outb(port + 1, 0x01);  
}


void serial_disable_interrupts(uint16_t port) {
    if (!serial_is_initialized(port)) return;
    
    outb(port + 1, 0x00);  
}
