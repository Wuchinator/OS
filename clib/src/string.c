#include "stddef.h"
#include "types.h"

size_t strlen(const char* str) {
    int len = 0;
    while(str[len]) {
        len++;
    }
    return len;
}


int strcmp(const char* s1, const char* s2) {

    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

char* strcpy(char* dest, const char* src) {
    char *ptr = dest;
    while (*src) {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
    return ptr;  
}


char* strcat(char* dest, const char* src) {
    size_t dest_len = strlen(dest);
    size_t src_len = strlen(src);
    for (size_t i = 0; i<src_len; i++) {
        dest[dest_len + i] = src[i];
    }
    dest[dest_len + src_len] = '\0';
    return dest;
}

char* strchr(const char* str, int c) {
    while (*str != '\0') {
        if (*str == (char)c) {
            return (char*)str;
        }
        str++;
    }
    if (c == '\0') {
        return (char*)str;
    }
    return NULL;
}

char* strstr(const char* haystack, const char* needle) {
    int flag = 0;
    int t = 0;

    if (NULL == needle || NULL == haystack) return NULL;
    if (*haystack == '\0' || *needle == '\0') {
        haystack = "";
        flag = 1;
    }

    if (flag == 0) {
        while (*haystack != '\0' && t == 0) {
            const char *h = haystack;
            const char *n = needle;
            while (*h != '\0' && *n != '\0' && *h == *n) {
                h++;
                n++;
            }

            if (*n == '\0') t = 1;
            if (t == 0) haystack++;
        }
        if (t == 0) haystack = NULL;
    }
    return (char *)haystack;
}