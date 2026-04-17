#ifndef STDLIB_H
#define STDLIB_H

#include "stddef.h"

void* malloc(size_t size);
void* calloc(size_t num, size_t size);
void* realloc(void* ptr, size_t new_size);
void free(void* ptr);

int atoi(const char* str);
long atol(const char* str);
long long atoll(const char* str);
double atof(const char* str);
long strtol(const char* str, char** endptr, int base);
unsigned long strtoul(const char* str, char** endptr, int base);

int abs(int j);
long labs(long j);
long long llabs(long long j);

int rand(void);
void srand(unsigned int seed);

void exit(int status);
int atexit(void (*function)(void));
void abort(void);

void* bsearch(const void* key, const void* base, size_t num, size_t size,
              int (*compar)(const void*, const void*));
void qsort(void* base, size_t num, size_t size,
           int (*compar)(const void*, const void*));

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define RAND_MAX 32767

#endif
