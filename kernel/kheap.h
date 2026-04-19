#ifndef KHEAP_H
#define KHEAP_H

#include "types.h"

#define KHEAP_START 0x00400000
#define KHEAP_END   0x04000000

void kheap_init(void);
void* kmalloc(uint32_t size);
void* kmalloc_align(uint32_t size, uint32_t alignment);
void kfree(void* ptr);
void kfree_align(void* ptr);
int kheap_is_initialized(void);
uint32_t kheap_get_used(void);
uint32_t kheap_get_reserved(void);
uint32_t kheap_get_limit(void);

#endif 
