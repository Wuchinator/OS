#ifndef KHEAP_H
#define KHEAP_H

#include "types.h"

void kheap_init(void);
void* kmalloc(uint32_t size);
void* kmalloc_align(uint32_t size, uint32_t alignment);
void kfree(void* ptr);
void kfree_align(void* ptr);
uint32_t kheap_get_used(void);

#endif 
