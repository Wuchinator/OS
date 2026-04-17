#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "stdint.h"


#define KEYBOARD_DATA_PORT    0x60
#define KEYBOARD_STATUS_PORT  0x64
#define KEYBOARD_COMMAND_PORT 0x64


#define KEYBOARD_STATUS_OUTPUT_FULL 0x01
#define KEYBOARD_STATUS_INPUT_FULL  0x02
#define KEYBOARD_STATUS_SYSTEM_FLAG 0x04


#define KEY_ESCAPE   0x01
#define KEY_BACKSPACE 0x0E
#define KEY_TAB       0x0F
#define KEY_ENTER     0x1C
#define KEY_CTRL      0x1D
#define KEY_SHIFT_L   0x2A
#define KEY_SHIFT_R   0x36
#define KEY_ALT       0x38
#define KEY_SPACE     0x39
#define KEY_CAPS_LOCK 0x3A
#define KEY_F1        0x3B
#define KEY_F2        0x3C
#define KEY_F3        0x3D
#define KEY_F4        0x3E
#define KEY_F5        0x3F
#define KEY_F6        0x40
#define KEY_F7        0x41
#define KEY_F8        0x42
#define KEY_F9        0x43
#define KEY_F10       0x44
#define KEY_NUM_LOCK  0x45
#define KEY_SCROLL_LOCK 0x46
#define KEY_HOME      0x47
#define KEY_UP        0x48
#define KEY_PAGE_UP   0x49
#define KEY_LEFT      0x4B
#define KEY_RIGHT     0x4D
#define KEY_END       0x4F
#define KEY_DOWN      0x50
#define KEY_PAGE_DOWN 0x51
#define KEY_INSERT    0x52
#define KEY_DELETE    0x53
#define KEYBOARD_BUFFER_SIZE 256

typedef struct {
    uint8_t scancode;
    char ascii;
    int pressed;
    int shift;
    int ctrl;
    int alt;
} key_event_t;

typedef struct {
    int shift_pressed;
    int ctrl_pressed;
    int alt_pressed;
    int caps_lock;
    int num_lock;
    int scroll_lock;
} keyboard_state_t;

void keyboard_init(void);

char keyboard_getchar(void);

int keyboard_has_data(void);

uint8_t keyboard_read_scancode(void);

int keyboard_get_event(key_event_t* event);

void keyboard_handler(void);

char keyboard_scancode_to_ascii(uint8_t scancode, int shift, int caps_lock);

#endif 
