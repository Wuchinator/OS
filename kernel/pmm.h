#ifndef PMM_H
#define PMM_H
#include "types.h"


#define PAGE_SIZE 4096
#define BITS_PER_UINT32 (sizeof(uint32_t) * 8)


void pmm_init(uint32_t memory_start, uint32_t memory_end);
void* pmm_alloc_page(void);
void pmm_free_page(void* page);
void* pmm_alloc_pages(uint32_t count);
void pmm_free_pages(void* page, uint32_t count);
uint32_t pmm_get_free_pages(void);
uint32_t pmm_get_total_pages(void);

#endif
