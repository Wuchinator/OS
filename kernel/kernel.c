#include "types.h"
#include "idt.h"
#include "isr.h"
#include "pmm.h"
#include "../drivers/vga.h"
#include "../drivers/serial.h"
#include "../drivers/timer.h"
#include "../drivers/keyboard.h"
#include "io.h"
#include "stddef.h"
#include "string.h"


extern uint32_t kernel_end;

#define TERMINAL_INPUT_MAX 128

static char terminal_input[TERMINAL_INPUT_MAX];
static uint32_t terminal_input_len = 0;


static void timer_irq_handler(struct interrupt_frame* frame) {
    (void)frame;
    timer_handler();
}

static void keyboard_irq_handler(struct interrupt_frame* frame) {
    (void)frame;
    keyboard_handler();
}

static void terminal_write(const char* text) {
    while (*text) {
        vga_putchar(*text);
        serial_write(COM1, (uint8_t)*text);
        text++;
    }
}

static void terminal_newline(void) {
    vga_putchar('\n');
    serial_write_string(COM1, "\r\n");
}

static void terminal_prompt(void) {
    terminal_write("os> ");
}

static void terminal_reset_input(void) {
    terminal_input_len = 0;
    terminal_input[0] = '\0';
}

static void terminal_print_banner(void) {
    terminal_write("Simple kernel terminal");
    terminal_newline();
    terminal_write("Type 'help' for commands.");
    terminal_newline();
}

static void terminal_execute_command(void) {
    terminal_input[terminal_input_len] = '\0';
    terminal_newline();

    if (terminal_input_len == 0) {
        terminal_prompt();
        return;
    }

    if (strcmp(terminal_input, "help") == 0) {
        terminal_write("Commands: help clear about");
        terminal_newline();
    } else if (strcmp(terminal_input, "clear") == 0) {
        vga_terminal_init();
    } else if (strcmp(terminal_input, "about") == 0) {
        terminal_write("Custom 32-bit hobby OS kernel");
        terminal_newline();
    } else {
        terminal_write("Unknown command: ");
        terminal_write(terminal_input);
        terminal_newline();
    }

    terminal_reset_input();
    terminal_prompt();
}

static void terminal_handle_key(const key_event_t* event) {
    if (event->scancode == KEY_BACKSPACE || event->ascii == '\b') {
        if (terminal_input_len > 0) {
            terminal_input_len--;
            terminal_input[terminal_input_len] = '\0';
            vga_putchar('\b');
            serial_write(COM1, '\b');
            serial_write(COM1, ' ');
            serial_write(COM1, '\b');
        }
        return;
    }

    if (event->scancode == KEY_ENTER || event->ascii == '\n') {
        terminal_execute_command();
        return;
    }

    if (event->ascii < 32 || event->ascii > 126) {
        return;
    }

    if (terminal_input_len >= (TERMINAL_INPUT_MAX - 1)) {
        return;
    }

    terminal_input[terminal_input_len++] = event->ascii;
    terminal_input[terminal_input_len] = '\0';
    vga_putchar(event->ascii);
    serial_write(COM1, (uint8_t)event->ascii);
}

static void terminal_run(void) {
    key_event_t event;

    vga_terminal_init();
    terminal_reset_input();
    terminal_print_banner();
    terminal_prompt();

    while (1) {
        if (keyboard_get_event(&event) != 0) {
            __asm__ volatile("hlt");
            continue;
        }

        terminal_handle_key(&event);
    }
}

void kernel_main(void) {
    __asm__ volatile("cli");
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
    serial_init(COM1, 115200);
    uint32_t mem_start = (uint32_t)&kernel_end;
    uint32_t mem_end = 0x400000;
    pmm_init(mem_start, mem_end);
    idt_init();
    pic_init();
    timer_init(100);
    irq_register_handler(0, timer_irq_handler);
    pic_unmask_irq(0);
    keyboard_init();
    irq_register_handler(1, keyboard_irq_handler);
    pic_unmask_irq(1);
    __asm__ volatile("sti");
    terminal_run();
}
