#ifndef VMM_H
#define VMM_H

#include "types.h"
#include "pmm.h"


#define PAGE_PRESENT    0x1
#define PAGE_WRITE      0x2
#define PAGE_USER       0x4
#define PAGE_WRITEBACK  0x8
#define PAGE_NOCACHE    0x10
#define PAGE_FRAME_MASK 0xFFFFF000

typedef struct {
    uint32_t entries[1024];
} __attribute__((aligned(PAGE_SIZE))) page_table_t;

typedef struct {
    uint32_t entries[1024];
} __attribute__((aligned(PAGE_SIZE))) page_directory_t;

extern page_directory_t* current_directory;
void vmm_init(void);
page_directory_t* vmm_create_directory(void);
void vmm_destroy_directory(page_directory_t* dir);
void vmm_switch_directory(page_directory_t* dir);
int vmm_map_page(page_directory_t* dir, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags);
int vmm_unmap_page(page_directory_t* dir, uint32_t virtual_addr);
uint32_t vmm_get_physical(page_directory_t* dir, uint32_t virtual_addr);
void* vmm_alloc_page(page_directory_t* dir, uint32_t virtual_addr, uint32_t flags);

#endif
