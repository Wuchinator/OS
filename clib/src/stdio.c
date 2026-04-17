#include "stdio.h"
#include "string.h"
#include "types.h"
#include "ctype.h"

static FILE _stdin  = { .fd = 0, .flags = 1 };
static FILE _stdout = { .fd = 1, .flags = 1 };
static FILE _stderr = { .fd = 2, .flags = 1 };

FILE* stdin  = &_stdin;
FILE* stdout = &_stdout;
FILE* stderr = &_stderr;

static char* itoa(int value, char* str, int base) {
    char* ptr = str;
    char* ptr2 = str;
    int tmp;
    int negative = 0;

    if (value == 0) {
        *str = '0';
        *(str + 1) = '\0';
        return str;
    }

    if (base == 10 && value < 0) {
        negative = 1;
        value = -value;
    }

    while (value > 0) {
        int rem = value % base;
        value = value / base;
        if (rem < 10)
            *ptr++ = rem + '0';
        else
            *ptr++ = rem - 10 + 'A';
    }

    if (negative)
        *ptr++ = '-';

    *ptr = '\0';
    ptr--;
    while (ptr2 < ptr) {
        tmp = *ptr2;
        *ptr2 = *ptr;
        *ptr = tmp;
        ptr2++;
        ptr--;
    }

    return str;
}


static char* utoa(unsigned int value, char* str, int base) {
    char* ptr = str;
    char* ptr2 = str;
    int tmp;

    if (value == 0) {
        *str = '0';
        *(str + 1) = '\0';
        return str;
    }

    const char* digits = "0123456789ABCDEF";
    
    while (value > 0) {
        *ptr++ = digits[value % base];
        value = value / base;
    }

    *ptr = '\0';
    ptr--;
    while (ptr2 < ptr) {
        tmp = *ptr2;
        *ptr2 = *ptr;
        *ptr = tmp;
        ptr2++;
        ptr--;
    }

    return str;
}

__attribute__((weak))
int kputchar(int c) {
    
    volatile char* vga_memory = (volatile char*)0xB8000;
    static int cursor_x = 0;
    static int cursor_y = 0;
    
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~7;
    } else {
        int offset = (cursor_y * 80 + cursor_x) * 2;
        vga_memory[offset] = (char)c;
        vga_memory[offset + 1] = 0x07; 
        cursor_x++;
    }
    
    if (cursor_x >= 80) {
        cursor_x = 0;
        cursor_y++;
    }
    
    
    if (cursor_y >= 25) {
        cursor_y = 24;
    }
    
    return c;
}

int putchar(int c) {
    return kputchar(c);
}

int fputc(int c, FILE* stream) {
    (void)stream; 
    return kputchar(c);
}

int putc(int c, FILE* stream) {
    return fputc(c, stream);
}

int puts(const char* str) {
    int count = 0;
    while (*str) {
        kputchar(*str);
        str++;
        count++;
    }
    kputchar('\n');
    count++;
    return count;
}

int fputs(const char* str, FILE* stream) {
    (void)stream;
    int count = 0;
    while (*str) {
        kputchar(*str);
        str++;
        count++;
    }
    return count;
}


int vprintf(const char* format, va_list args) {
    int count = 0;
    char buffer[256];
    
    while (*format) {
        if (*format == '%') {
            format++;
            
            
            int width = 0;
            while (*format >= '0' && *format <= '9') {
                width = width * 10 + (*format - '0');
                format++;
            }
            
            switch (*format) {
                case 'd':
                case 'i': {
                    int val = va_arg(args, int);
                    itoa(val, buffer, 10);
                    count += fputs(buffer, stdout);
                    break;
                }
                case 'u': {
                    unsigned int val = va_arg(args, unsigned int);
                    utoa(val, buffer, 10);
                    count += fputs(buffer, stdout);
                    break;
                }
                case 'x': {
                    unsigned int val = va_arg(args, unsigned int);
                    utoa(val, buffer, 16);
                    count += fputs(buffer, stdout);
                    break;
                }
                case 'X': {
                    unsigned int val = va_arg(args, unsigned int);
                    utoa(val, buffer, 16);
                    count += fputs(buffer, stdout);
                    break;
                }
                case 'p': {
                    void* val = va_arg(args, void*);
                    fputs("0x", stdout);
                    utoa((unsigned int)val, buffer, 16);
                    count += 2 + fputs(buffer, stdout);
                    break;
                }
                case 'c': {
                    int val = va_arg(args, int);
                    kputchar(val);
                    count++;
                    break;
                }
                case 's': {
                    char* val = va_arg(args, char*);
                    if (!val) val = "(null)";
                    count += fputs(val, stdout);
                    break;
                }
                case '%': {
                    kputchar('%');
                    count++;
                    break;
                }
                default: {
                    kputchar('%');
                    kputchar(*format);
                    count += 2;
                    break;
                }
            }
        } else {
            kputchar(*format);
            count++;
        }
        format++;
    }
    
    return count;
}

int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int count = vprintf(format, args);
    va_end(args);
    return count;
}

int fprintf(FILE* stream, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    int count = 0;
    char buffer[256];
    
    while (*format) {
        if (*format == '%') {
            format++;
            
            int width = 0;
            while (*format >= '0' && *format <= '9') {
                width = width * 10 + (*format - '0');
                format++;
            }
            
            switch (*format) {
                case 'd':
                case 'i': {
                    int val = va_arg(args, int);
                    itoa(val, buffer, 10);
                    count += fputs(buffer, stream);
                    break;
                }
                case 'u': {
                    unsigned int val = va_arg(args, unsigned int);
                    utoa(val, buffer, 10);
                    count += fputs(buffer, stream);
                    break;
                }
                case 'x': {
                    unsigned int val = va_arg(args, unsigned int);
                    utoa(val, buffer, 16);
                    count += fputs(buffer, stream);
                    break;
                }
                case 'c': {
                    int val = va_arg(args, int);
                    fputc(val, stream);
                    count++;
                    break;
                }
                case 's': {
                    char* val = va_arg(args, char*);
                    if (!val) val = "(null)";
                    count += fputs(val, stream);
                    break;
                }
                case '%': {
                    fputc('%', stream);
                    count++;
                    break;
                }
                default: {
                    fputc('%', stream);
                    fputc(*format, stream);
                    count += 2;
                    break;
                }
            }
        } else {
            fputc(*format, stream);
            count++;
        }
        format++;
    }
    
    va_end(args);
    return count;
}

int sprintf(char* str, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    int count = 0;
    char buffer[256];
    
    while (*format) {
        if (*format == '%') {
            format++;
            
            int width = 0;
            while (*format >= '0' && *format <= '9') {
                width = width * 10 + (*format - '0');
                format++;
            }
            
            switch (*format) {
                case 'd':
                case 'i': {
                    int val = va_arg(args, int);
                    char* result = itoa(val, buffer, 10);
                    while (*result) {
                        *str++ = *result++;
                        count++;
                    }
                    break;
                }
                case 'u': {
                    unsigned int val = va_arg(args, unsigned int);
                    char* result = utoa(val, buffer, 10);
                    while (*result) {
                        *str++ = *result++;
                        count++;
                    }
                    break;
                }
                case 'x': {
                    unsigned int val = va_arg(args, unsigned int);
                    char* result = utoa(val, buffer, 16);
                    while (*result) {
                        *str++ = *result++;
                        count++;
                    }
                    break;
                }
                case 'c': {
                    int val = va_arg(args, int);
                    *str++ = (char)val;
                    count++;
                    break;
                }
                case 's': {
                    char* val = va_arg(args, char*);
                    if (!val) val = "(null)";
                    while (*val) {
                        *str++ = *val++;
                        count++;
                    }
                    break;
                }
                case '%': {
                    *str++ = '%';
                    count++;
                    break;
                }
                default: {
                    *str++ = '%';
                    *str++ = *format;
                    count += 2;
                    break;
                }
            }
        } else {
            *str++ = *format;
            count++;
        }
        format++;
    }
    
    *str = '\0';
    va_end(args);
    return count;
}

int snprintf(char* str, size_t size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    int count = 0;
    char buffer[256];
    size_t max_len = size - 1; 
    
    while (*format && count < (int)max_len) {
        if (*format == '%') {
            format++;
            
            int width = 0;
            while (*format >= '0' && *format <= '9') {
                width = width * 10 + (*format - '0');
                format++;
            }
            
            switch (*format) {
                case 'd':
                case 'i': {
                    int val = va_arg(args, int);
                    char* result = itoa(val, buffer, 10);
                    while (*result && count < (int)max_len) {
                        *str++ = *result++;
                        count++;
                    }
                    break;
                }
                case 'u': {
                    unsigned int val = va_arg(args, unsigned int);
                    char* result = utoa(val, buffer, 10);
                    while (*result && count < (int)max_len) {
                        *str++ = *result++;
                        count++;
                    }
                    break;
                }
                case 'x': {
                    unsigned int val = va_arg(args, unsigned int);
                    char* result = utoa(val, buffer, 16);
                    while (*result && count < (int)max_len) {
                        *str++ = *result++;
                        count++;
                    }
                    break;
                }
                case 'c': {
                    int val = va_arg(args, int);
                    *str++ = (char)val;
                    count++;
                    break;
                }
                case 's': {
                    char* val = va_arg(args, char*);
                    if (!val) val = "(null)";
                    while (*val && count < (int)max_len) {
                        *str++ = *val++;
                        count++;
                    }
                    break;
                }
                case '%': {
                    *str++ = '%';
                    count++;
                    break;
                }
                default: {
                    *str++ = '%';
                    *str++ = *format;
                    count += 2;
                    break;
                }
            }
        } else {
            *str++ = *format;
            count++;
        }
        format++;
    }
    
    *str = '\0';
    va_end(args);
    return count;
}


__attribute__((weak))
int kgetchar(void) {
    
    return EOF;
}

int getchar(void) {
    return kgetchar();
}

int fgetc(FILE* stream) {
    (void)stream;
    return kgetchar();
}

int getc(FILE* stream) {
    return fgetc(stream);
}

char* fgets(char* str, int size, FILE* stream) {
    int i = 0;
    while (i < size - 1) {
        int c = fgetc(stream);
        if (c == EOF) {
            break;
        }
        str[i++] = (char)c;
        if (c == '\n') {
            break;
        }
    }
    str[i] = '\0';
    return (i > 0) ? str : NULL;
}

int vfprintf(FILE* stream, const char* format, va_list args) {
    
    (void)stream;
    return vprintf(format, args);
}
