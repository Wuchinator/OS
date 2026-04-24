#include "vmm.h"
#include "pmm.h"
#include "types.h"
#include "stddef.h"
#include "string.h"

page_directory_t* current_directory = NULL;

static page_table_t* vmm_get_page_table(page_directory_t* dir, uint32_t virtual_addr, int create) {
    uint32_t pd_index = (virtual_addr >> 22) & 0x3FF;
    uint32_t entry = dir->entries[pd_index];

    if (entry & PAGE_PRESENT) {
        return (page_table_t*)(entry & PAGE_FRAME_MASK);
    }

    if (!create) {
        return NULL;
    }

    void* table_phys = pmm_alloc_page();
    if (!table_phys) {
        return NULL;
    }

    page_table_t* table = (page_table_t*)table_phys;
    memset(table, 0, sizeof(page_table_t));
    dir->entries[pd_index] = ((uint32_t)table_phys & PAGE_FRAME_MASK) | PAGE_PRESENT | PAGE_WRITE;

    return table;
}

int vmm_map_page(page_directory_t* dir, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;
    page_table_t* table = vmm_get_page_table(dir, virtual_addr, 1);
    if (!table) {
        return -1;
    }
    if (table->entries[pt_index] & PAGE_PRESENT) {
        return -1;
    }
    table->entries[pt_index] = (physical_addr & PAGE_FRAME_MASK) | (flags & ~PAGE_FRAME_MASK);

    if (dir == current_directory) {
        __asm__ volatile("invlpg (%0)" : : "r"(virtual_addr) : "memory");
    }

    return 0;
}

int vmm_unmap_page(page_directory_t* dir, uint32_t virtual_addr) {
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;

    page_table_t* table = vmm_get_page_table(dir, virtual_addr, 0);
    if (!table) {
        return -1;
    }

    table->entries[pt_index] = 0;
    __asm__ volatile("invlpg (%0)" : : "r"(virtual_addr) : "memory");

    return 0;
}

uint32_t vmm_get_physical(page_directory_t* dir, uint32_t virtual_addr) {
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;
    uint32_t entry;

    page_table_t* table = vmm_get_page_table(dir, virtual_addr, 0);
    if (!table) {
        return 0;
    }

    entry = table->entries[pt_index];
    if (!(entry & PAGE_PRESENT)) {
        return 0;
    }

    uint32_t page_frame = entry & PAGE_FRAME_MASK;
    uint32_t offset = virtual_addr & 0xFFF;

    return page_frame + offset;
}

void* vmm_alloc_page(page_directory_t* dir, uint32_t virtual_addr, uint32_t flags) {
    void* physical = pmm_alloc_page();
    if (!physical) {
        return NULL;
    }
    if (vmm_map_page(dir, virtual_addr, (uint32_t)physical, flags) != 0) {
        pmm_free_page(physical);
        return NULL;
    }

    return physical;
}

page_directory_t* vmm_create_directory(void) {
    void* pd_phys = pmm_alloc_page();
    if (!pd_phys) {
        return NULL;
    }

    page_directory_t* dir = (page_directory_t*)pd_phys;
    memset(dir, 0, sizeof(page_directory_t));

    return dir;
}

void vmm_destroy_directory(page_directory_t* dir) {
    if (!dir || dir == current_directory) {
        return;
    }
    for (int i = 0; i < 1024; i++) {
        if (dir->entries[i] & PAGE_PRESENT) {
            page_table_t* table = (page_table_t*)(dir->entries[i] & PAGE_FRAME_MASK);
            pmm_free_page(table);
        }
    }
    pmm_free_page(dir);
}

void vmm_switch_directory(page_directory_t* dir) {
    current_directory = dir;
    uint32_t cr3_value = (uint32_t)dir;
    __asm__ volatile("mov %0, %%cr3" : : "r"(cr3_value) : "memory");
}

void vmm_init(void) {
    page_directory_t* kernel_dir = vmm_create_directory();
    if (!kernel_dir) {
        return;
    }

    
    for (uint32_t addr = 0; addr < 0x00400000; addr += PAGE_SIZE) {
        vmm_map_page(kernel_dir, addr, addr, PAGE_PRESENT | PAGE_WRITE);
    }
    vmm_map_page(kernel_dir, 0xB8000, 0xB8000, PAGE_PRESENT | PAGE_WRITE);
    vmm_switch_directory(kernel_dir);

    
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0) : "memory");
}
