#include "vga.h"
#include "string.h"

static int terminal_row = 0;
static int terminal_col = 0;
static uint8_t terminal_color = VGA_LIGHT_GRAY | (VGA_BLACK << 4);
static volatile uint16_t* vga_buffer = (volatile uint16_t*)VGA_MEMORY;

uint8_t vga_entry_color(uint8_t fg, uint8_t bg) {
    return fg | bg << 4;
}

void vga_enable_cursor(int start, int end) {
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)start), "Nd"((uint16_t)0x3D4));
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)end), "Nd"((uint16_t)0x3D5));
}

void vga_disable_cursor(void) {
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x0A), "Nd"((uint16_t)0x3D4));
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x20), "Nd"((uint16_t)0x3D5));
}

void vga_enable_default_cursor(void) {
    vga_enable_cursor(14, 15);
}

void vga_set_cursor(int x, int y) {
    uint16_t pos = y * VGA_WIDTH + x;
    
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x0F), "Nd"((uint16_t)0x3D4));
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)(pos & 0xFF)), "Nd"((uint16_t)0x3D5));
    
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x0E), "Nd"((uint16_t)0x3D4));
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)((pos >> 8) & 0xFF)), "Nd"((uint16_t)0x3D5));
    
    terminal_row = y;
    terminal_col = x;
}

void vga_get_cursor(int* x, int* y) {
    if (x) *x = terminal_col;
    if (y) *y = terminal_row;
}

void vga_terminal_init(void) {
    terminal_row = 0;
    terminal_col = 0;
    terminal_color = vga_entry_color(VGA_LIGHT_GRAY, VGA_BLACK);
    vga_clear_screen();
    vga_enable_default_cursor();
}

void vga_clear_screen(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = ' ' | terminal_color << 8;
    }
    terminal_row = 0;
    terminal_col = 0;
    vga_set_cursor(0, 0);
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    terminal_color = vga_entry_color(fg, bg);
}

void vga_scroll(void) {
    vga_scroll_up(1);
}

void vga_scroll_up(int lines) {
    if (lines >= VGA_HEIGHT) {
        vga_clear_screen();
        return;
    }
    
    for (int y = lines; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH - lines * VGA_WIDTH + x] = vga_buffer[y * VGA_WIDTH + x];
        }
    }
    
    for (int y = VGA_HEIGHT - lines; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = ' ' | terminal_color << 8;
        }
    }
    
    terminal_row -= lines;
    if (terminal_row < 0) terminal_row = 0;
}

static void vga_put_entry(char c, uint8_t color, int x, int y) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        vga_buffer[y * VGA_WIDTH + x] = c | color << 8;
    }
}

void vga_putchar(char c) {
    if (c == '\n') {
        terminal_col = 0;
        terminal_row++;
    } else if (c == '\r') {
        terminal_col = 0;
    } else if (c == '\t') {
        terminal_col = (terminal_col + 8) & ~7;
        if (terminal_col >= VGA_WIDTH) {
            terminal_col = 0;
            terminal_row++;
        }
    } else if (c == '\b') {
        if (terminal_col > 0) {
            terminal_col--;
            vga_put_entry(' ', terminal_color, terminal_col, terminal_row);
        }
    } else {
        vga_put_entry(c, terminal_color, terminal_col, terminal_row);
        terminal_col++;
    }
    
    if (terminal_col >= VGA_WIDTH) {
        terminal_col = 0;
        terminal_row++;
    }
    
    if (terminal_row >= VGA_HEIGHT) {
        vga_scroll();
    }
    
    vga_set_cursor(terminal_col, terminal_row);
}

void vga_putstring(const char* str) {
    while (*str) {
        vga_putchar(*str);
        str++;
    }
}

void vga_print_char(char c, int x, int y, uint8_t color) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        vga_put_entry(c, color, x, y);
    }
}

void vga_print_string(const char* str, int x, int y, uint8_t color) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        while (*str && x < VGA_WIDTH) {
            vga_put_entry(*str, color, x, y);
            str++;
            x++;
        }
    }
}

void vga_putchar_at(char c, int x, int y, uint8_t color) {
    vga_put_entry(c, color, x, y);
}

void vga_fill_rect(int x, int y, int width, int height, char c, uint8_t color) {
    for (int row = y; row < y + height && row < VGA_HEIGHT; row++) {
        for (int col = x; col < x + width && col < VGA_WIDTH; col++) {
            vga_put_entry(c, color, col, row);
        }
    }
}

__attribute__((weak))
int kputchar(int c) {
    vga_putchar((char)c);
    return c;
}

