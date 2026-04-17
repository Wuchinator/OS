#include "keyboard.h"
#include "string.h"
#include "io.h"


static volatile uint8_t keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static volatile int buffer_head = 0;
static volatile int buffer_tail = 0;
static keyboard_state_t kb_state = {0};


static const char keyboard_layout_us[128] = {
    0,    0,   '1', '2', '3', '4', '5', '6',  
    '7', '8', '9', '0', '-', '=', '\b', '\t',  
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',   
    'o', 'p', '[', ']', '\n', 0,    'a', 's',  
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',   
    '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',   
    'b', 'n', 'm', ',', '.', '/', 0, '*',     
    0,   ' ', 0,   0,   0,   0,   0,   0,     
    0,   0,   0,   0,   0,   0,   0,   '7',   
    '8', '9', '-', '4', '5', '6', '+', '1',   
    '2', '3', '0', '.', 0,   0,   0,   0,     
    0,   0,   0,   0,   0,   0,   0,   0,     
    0,   0,   0,   0,   0,   0,   0,   0,     
    0,   0,   0,   0,   0,   0,   0,   0,     
    0,   0,   0,   0,   0,   0,   0,   0,     
    0,   0,   0,   0,   0,   0,   0,   0      
};


static const char keyboard_layout_us_shift[128] = {
    0,    0,   '!', '@', '#', '$', '%', '^',  
    '&', '*', '(', ')', '_', '+', '\b', '\t',  
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',   
    'O', 'P', '{', '}', '\n', 0,    'A', 'S',  
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',   
    '"', '~', 0, '|', 'Z', 'X', 'C', 'V',     
    'B', 'N', 'M', '<', '>', '?', 0, '*',     
    0,   ' ', 0,   0,   0,   0,   0,   0,     
    0,   0,   0,   0,   0,   0,   0,   '7',   
    '8', '9', '-', '4', '5', '6', '+', '1',   
    '2', '3', '0', '.', 0,   0,   0,   0,     
    0,   0,   0,   0,   0,   0,   0,   0,     
    0,   0,   0,   0,   0,   0,   0,   0,     
    0,   0,   0,   0,   0,   0,   0,   0,     
    0,   0,   0,   0,   0,   0,   0,   0,     
    0,   0,   0,   0,   0,   0,   0,   0      
};

void keyboard_init(void) {
    buffer_head = 0;
    buffer_tail = 0;
    memset((void*)&kb_state, 0, sizeof(keyboard_state_t));
    
    
    while (inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_INPUT_FULL) {
        inb(KEYBOARD_DATA_PORT);
    }
}


static uint8_t keyboard_read_data(void) {
    return inb(KEYBOARD_DATA_PORT);
}


int keyboard_has_data(void) {
    return (inb(KEYBOARD_STATUS_PORT) & KEYBOARD_STATUS_OUTPUT_FULL) != 0;
}

uint8_t keyboard_read_scancode(void) {
    if (!keyboard_has_data()) {
        return 0;
    }
    return keyboard_read_data();
}


char keyboard_scancode_to_ascii(uint8_t scancode, int shift, int caps_lock) {
    if (scancode >= 128) return 0;
    
    char c = shift ? keyboard_layout_us_shift[scancode] : keyboard_layout_us[scancode];
    
    if (caps_lock && c >= 'a' && c <= 'z') {
        return c - 32;
    }
    if (caps_lock && c >= 'A' && c <= 'Z') {
        return c + 32;
    }
    
    return c;
}

static void keyboard_buffer_add(uint8_t scancode) {
    int next_tail = (buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;
    
    
    if (next_tail != buffer_head) {
        keyboard_buffer[buffer_tail] = scancode;
        buffer_tail = next_tail;
    }
}

static uint8_t keyboard_buffer_get(void) {
    if (buffer_head == buffer_tail) {
        return 0;
    }
    
    uint8_t scancode = keyboard_buffer[buffer_head];
    buffer_head = (buffer_head + 1) % KEYBOARD_BUFFER_SIZE;
    return scancode;
}

void keyboard_handler(void) {
    uint8_t scancode = keyboard_read_data();
    if (scancode == 0) return;
    int released = (scancode & 0x80) != 0;
    uint8_t key = scancode & 0x7F;
    switch (key) {
        case KEY_SHIFT_L:
        case KEY_SHIFT_R:
            kb_state.shift_pressed = !released;
            return;
        case KEY_CTRL:
            kb_state.ctrl_pressed = !released;
            return;
        case KEY_ALT:
            kb_state.alt_pressed = !released;
            return;
        case KEY_CAPS_LOCK:
            if (!released) {
                kb_state.caps_lock = !kb_state.caps_lock;
            }
            return;
    }
    
    if (!released) {
        keyboard_buffer_add(scancode);
    }
}


int keyboard_get_event(key_event_t* event) {
    if (!event) return -1;
    
    uint8_t scancode = keyboard_buffer_get();
    if (scancode == 0) return -1;
    
    event->scancode = scancode;
    event->pressed = 1;
    event->shift = kb_state.shift_pressed;
    event->ctrl = kb_state.ctrl_pressed;
    event->alt = kb_state.alt_pressed;
    
    
    event->ascii = keyboard_scancode_to_ascii(scancode, kb_state.shift_pressed, kb_state.caps_lock);
    
    return 0;
}


char keyboard_getchar(void) {
    while (buffer_head == buffer_tail) {
        
        __asm__ volatile("hlt");
    }
    
    uint8_t scancode = keyboard_buffer_get();
    return keyboard_scancode_to_ascii(scancode, kb_state.shift_pressed, kb_state.caps_lock);
}


__attribute__((weak))
int kgetchar(void) {
    return keyboard_getchar();
}
