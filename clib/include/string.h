#ifndef STRING_H
#define STRING_H

#include "stddef.h"

void* memcpy(void* dest, const void* src, size_t n);
void* memmove(void* dest, const void* src, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
void* memset(void* ptr, int value, size_t num); 
size_t strlen(const char* str);
int strcmp(const char* s1, const char* s2);
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src);
char* strchr(const char* str, int c);
char* strstr(const char* haystack, const char* needle);

#endif 