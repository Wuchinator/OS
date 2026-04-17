#ifndef SERIAL_H
#define SERIAL_H

#include "stdint.h"


#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8



int serial_init(uint16_t port, int baud_rate);


int serial_is_initialized(uint16_t port);


int serial_read(uint16_t port);


void serial_write(uint16_t port, uint8_t data);


void serial_write_string(uint16_t port, const char* str);


void serial_write_hex(uint16_t port, uint32_t value);


void serial_write_dec(uint16_t port, int32_t value);


int serial_data_available(uint16_t port);


void serial_enable_interrupts(uint16_t port);


void serial_disable_interrupts(uint16_t port);

#endif 
