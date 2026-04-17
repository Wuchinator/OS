#include "stddef.h"

void *memchr(const void *str, int c, size_t n) {
    unsigned char *memptr = (unsigned char *)str;
    void *out = NULL;
    for (size_t i = 0; i < n && memptr; i++) {
        if ((*memptr) == (unsigned char)c) {
            out = memptr;
            break;
        }
        memptr++;
    }
    return out;
}

int memcmp(const void *str1, const void *str2, size_t n) {
    unsigned char *memptr1 = (unsigned char *)str1;
    unsigned char *memptr2 = (unsigned char *)str2;
    int result = 0, flag = 0;
    for (size_t i = 0; flag != 1 && i < n; i++) {
        result = *memptr1 - *memptr2;
        if (*memptr1 != *memptr2) {
            flag = 1;
        }
        memptr1++;
        memptr2++;
    }
    return result;
}


void* memmove(void* dest, const void* src, size_t n) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    if (d == s || n == 0) return dest;
    if (d < s) {
        for (size_t i = 0; i < n; i++) {
            d[i] = s[i];
        }
    } else {
        for (size_t i = n; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    }
    return dest;
}

void *memcpy(void *dest, const void *src, size_t n) {
  unsigned char *destination = (unsigned char *)dest;
  unsigned char *source = (unsigned char *)src;
  for (size_t i = 0; i < n; i++) destination[i] = source[i];
  return dest;
}

void *memset(void *str, int c, size_t n) {
  unsigned char *string = (unsigned char *)str;
  for (size_t i = 0; i < n; i++) string[i] = c;
  return str;
}