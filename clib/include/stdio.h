#ifndef STDIO_H
#define STDIO_H

#include "stdarg.h"
#include "stddef.h"

typedef struct {
    int fd;
    int flags;
} FILE;

#define EOF (-1)
#define BUFSIZ 512


extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

int printf(const char* format, ...);
int vprintf(const char* format, va_list args);
int fprintf(FILE* stream, const char* format, ...);
int vfprintf(FILE* stream, const char* format, va_list args);
int sprintf(char* str, const char* format, ...);
int snprintf(char* str, size_t size, const char* format, ...);

int putchar(int c);
int fputc(int c, FILE* stream);
int putc(int c, FILE* stream);
int puts(const char* str);
int fputs(const char* str, FILE* stream);
int getchar(void);
int fgetc(FILE* stream);
int getc(FILE* stream);
char* fgets(char* str, int size, FILE* stream);

int kputchar(int c);
int kgetchar(void);

#endif
