#include "types.h"
#include "idt.h"
#include "isr.h"
#include "pmm.h"
#include "vmm.h"
#include "kheap.h"
#include "../drivers/vga.h"
#include "../drivers/serial.h"
#include "../drivers/timer.h"
#include "../drivers/keyboard.h"
#include "io.h"
#include "stddef.h"
#include "stdio.h"
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

static void terminal_write_line(const char* text) {
    terminal_write(text);
    terminal_newline();
}

static void terminal_prompt(void) {
    terminal_write("os> ");
}

static void terminal_reset_input(void) {
    terminal_input_len = 0;
    terminal_input[0] = '\0';
}

static void terminal_print_banner(void) {
    terminal_write_line("Simple kernel terminal");
    terminal_write_line("Memory subsystem online.");
    terminal_write_line("Type 'help' for commands.");
}

static void kernel_panic(const char* message) {
    vga_terminal_init();
    terminal_write("KERNEL PANIC: ");
    terminal_write_line(message);
    __asm__ volatile("cli");

    while (1) {
        __asm__ volatile("hlt");
    }
}

static int memory_self_test(void) {
    uint8_t* small = NULL;
    uint8_t* medium = NULL;
    void* aligned = NULL;

    small = (uint8_t*)kmalloc(64);
    medium = (uint8_t*)kmalloc(512);
    aligned = kmalloc_align(256, PAGE_SIZE);

    if (!small || !medium || !aligned) {
        goto fail;
    }

    memset(small, 0xA5, 64);
    memset(medium, 0x5A, 512);
    memset(aligned, 0x3C, 256);

    if (((uint32_t)aligned & (PAGE_SIZE - 1)) != 0) {
        goto fail;
    }

    for (uint32_t i = 0; i < 64; i++) {
        if (small[i] != 0xA5) {
            goto fail;
        }
    }

    for (uint32_t i = 0; i < 512; i++) {
        if (medium[i] != 0x5A) {
            goto fail;
        }
    }

    for (uint32_t i = 0; i < 256; i++) {
        if (((uint8_t*)aligned)[i] != 0x3C) {
            goto fail;
        }
    }

    kfree(small);
    kfree(medium);
    kfree_align(aligned);
    return 0;

fail:
    if (small) {
        kfree(small);
    }

    if (medium) {
        kfree(medium);
    }

    if (aligned) {
        kfree_align(aligned);
    }

    return -1;
}

static void memory_init(void) {
    uint32_t mem_start = (uint32_t)&kernel_end;
    uint32_t mem_end = 0x400000;

    pmm_init(mem_start, mem_end);
    vmm_init();
    if (!current_directory) {
        kernel_panic("paging init failed");
    }

    kheap_init();
    if (!kheap_is_initialized()) {
        kernel_panic("kernel heap init failed");
    }

    if (memory_self_test() != 0) {
        kernel_panic("memory self-test failed");
    }
}

static void terminal_print_memory_status(void) {
    char line[128];
    uint32_t total_pages = pmm_get_total_pages();
    uint32_t free_pages = pmm_get_free_pages();
    uint32_t heap_base_phys = 0;

    snprintf(line, sizeof(line),
             "PMM: total=%u pages (%u KiB), free=%u pages (%u KiB)",
             total_pages,
             total_pages * 4,
             free_pages,
             free_pages * 4);
    terminal_write_line(line);

    snprintf(line, sizeof(line),
             "Paging: on, page directory=0x%x",
             (uint32_t)current_directory);
    terminal_write_line(line);

    snprintf(line, sizeof(line),
             "Heap: used=%u bytes, mapped=%u bytes, limit=%u bytes",
             kheap_get_used(),
             kheap_get_reserved(),
             kheap_get_limit());
    terminal_write_line(line);

    heap_base_phys = vmm_get_physical(current_directory, KHEAP_START);
    snprintf(line, sizeof(line),
             "Heap base: virt=0x%x phys=0x%x",
             KHEAP_START,
             heap_base_phys);
    terminal_write_line(line);
}

static void terminal_execute_command(void) {
    terminal_input[terminal_input_len] = '\0';
    terminal_newline();

    if (terminal_input_len == 0) {
        terminal_prompt();
        return;
    }

    if (strcmp(terminal_input, "help") == 0) {
        terminal_write_line("Commands:\n help\n clear\n about\n mem\n heaptest");
    } else if (strcmp(terminal_input, "clear") == 0) {
        vga_terminal_init();
    } else if (strcmp(terminal_input, "about") == 0) {
        terminal_write_line("Custom 32-bit hobby OS kernel");
    } else if (strcmp(terminal_input, "mem") == 0) {
        terminal_print_memory_status();
    } else if (strcmp(terminal_input, "heaptest") == 0) {
        if (memory_self_test() == 0) {
            terminal_write_line("Heap self-test passed.");
        } else {
            terminal_write_line("Heap self-test failed.");
        }
    } else {
        terminal_write("Unknown command: ");
        terminal_write_line(terminal_input);
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
    idt_init();
    memory_init();
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
