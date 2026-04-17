#ifndef VGA_H
#define VGA_H

#include "stdint.h"

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

typedef enum {
    VGA_BLACK         = 0x00,
    VGA_BLUE          = 0x01,
    VGA_GREEN         = 0x02,
    VGA_CYAN          = 0x03,
    VGA_RED           = 0x04,
    VGA_MAGENTA       = 0x05,
    VGA_BROWN         = 0x06,
    VGA_LIGHT_GRAY    = 0x07,
    VGA_DARK_GRAY     = 0x08,
    VGA_LIGHT_BLUE    = 0x09,
    VGA_LIGHT_GREEN   = 0x0A,
    VGA_LIGHT_CYAN    = 0x0B,
    VGA_LIGHT_RED     = 0x0C,
    VGA_LIGHT_MAGENTA = 0x0D,
    VGA_YELLOW        = 0x0E,
    VGA_WHITE         = 0x0F
} vga_color_t;

uint8_t vga_entry_color(uint8_t fg, uint8_t bg);
void vga_terminal_init(void);
void vga_clear_screen(void);
void vga_set_color(uint8_t fg, uint8_t bg);
void vga_putchar(char c);
void vga_putstring(const char* str);
void vga_print_char(char c, int x, int y, uint8_t color);
void vga_print_string(const char* str, int x, int y, uint8_t color);
void vga_set_cursor(int x, int y);
void vga_get_cursor(int* x, int* y);
void vga_enable_cursor(int start, int end);
void vga_disable_cursor(void);
void vga_scroll(void);
void vga_scroll_up(int lines);
void vga_putchar_at(char c, int x, int y, uint8_t color);
void vga_fill_rect(int x, int y, int width, int height, char c, uint8_t color);

#endif 
