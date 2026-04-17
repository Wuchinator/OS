#include "pmm.h"
#include "types.h"
#include "stddef.h"

static uint32_t* bitmap = NULL;
static uint32_t total_pages = 0;
static uint32_t free_pages = 0;
static uint32_t memory_start = 0;

static int bitmap_test(uint32_t index) {
    uint32_t bit_index = index % BITS_PER_UINT32;
    uint32_t byte_index = index / BITS_PER_UINT32;
    return (bitmap[byte_index] & (1 << bit_index)) != 0;
}

static void bitmap_set(uint32_t index) {
    uint32_t bit_index = index % BITS_PER_UINT32;
    uint32_t byte_index = index / BITS_PER_UINT32;
    bitmap[byte_index] |= (1 << bit_index);
}

static void bitmap_clear(uint32_t index) {
    uint32_t bit_index = index % BITS_PER_UINT32;
    uint32_t byte_index = index / BITS_PER_UINT32;
    bitmap[byte_index] &= ~(1 << bit_index);
}

static int32_t bitmap_first_free(void) {
    for (uint32_t i = 0; i < (total_pages + BITS_PER_UINT32 - 1) / BITS_PER_UINT32; i++) {
        if (bitmap[i] == 0xFFFFFFFF) {
            continue;
        }
        for (uint32_t j = 0; j < BITS_PER_UINT32; j++) {
            uint32_t index = i * BITS_PER_UINT32 + j;
            if (index >= total_pages) {
                return -1;
            }
            if (!bitmap_test(index)) {
                return (int32_t)index;
            }
        }
    }
    return -1;
}

void pmm_init(uint32_t mstart, uint32_t mend) {
    if (mend <= mstart) {
        memory_start = 0;
        total_pages = 0;
        free_pages = 0;
        bitmap = NULL;
        return;
    }

    memory_start = mstart;
    total_pages = (mend - mstart) / PAGE_SIZE;
    free_pages = total_pages;
    bitmap = (uint32_t*)mstart;

    uint32_t bitmap_size = (total_pages + BITS_PER_UINT32 - 1) / BITS_PER_UINT32;
    for (uint32_t i = 0; i < bitmap_size; i++) {
        bitmap[i] = 0;
    }

    
    uint32_t bitmap_bytes = bitmap_size * sizeof(uint32_t);
    uint32_t reserved_pages = (bitmap_bytes + PAGE_SIZE - 1) / PAGE_SIZE;
    if (reserved_pages == 0) {
        reserved_pages = 1;
    }

    for (uint32_t i = 0; i < reserved_pages && i < total_pages; i++) {
        bitmap_set(i);
        free_pages--;
    }
}

void* pmm_alloc_page(void) {
    if (!bitmap || total_pages == 0) {
        return NULL;
    }

    int32_t index = bitmap_first_free();
    if (index < 0) {
        return NULL; 
    }

    bitmap_set(index);
    free_pages--;

    return (void*)(memory_start + (index * PAGE_SIZE));
}

void pmm_free_page(void* page) {
    if (!bitmap || total_pages == 0) {
        return;
    }

    if (page == NULL || (uint32_t)page < memory_start) {
        return;
    }

    uint32_t offset = (uint32_t)page - memory_start;
    uint32_t index = offset / PAGE_SIZE;

    if (index < total_pages && bitmap_test(index)) {
        bitmap_clear(index);
        free_pages++;
    }
}

void* pmm_alloc_pages(uint32_t count) {
    if (!bitmap || total_pages == 0 || count == 0) return NULL;

    uint32_t found_start = 0;
    uint32_t consecutive = 0;

    for (uint32_t i = 0; i < total_pages; i++) {
        if (!bitmap_test(i)) {
            if (consecutive == 0) {
                found_start = i;
            }
            consecutive++;
            if (consecutive == count) {
                for (uint32_t j = 0; j < count; j++) {
                    bitmap_set(found_start + j);
                }
                free_pages -= count;
                return (void*)(memory_start + (found_start * PAGE_SIZE));
            }
        } else {
            consecutive = 0;
        }
    }

    return NULL;
}

void pmm_free_pages(void* page, uint32_t count) {
    if (!bitmap || total_pages == 0) {
        return;
    }

    if (page == NULL || count == 0 || (uint32_t)page < memory_start) {
        return;
    }

    uint32_t offset = (uint32_t)page - memory_start;
    uint32_t start_index = offset / PAGE_SIZE;

    for (uint32_t i = 0; i < count; i++) {
        uint32_t index = start_index + i;
        if (index < total_pages && bitmap_test(index)) {
            bitmap_clear(index);
            free_pages++;
        }
    }
}

uint32_t pmm_get_free_pages(void) {
    return free_pages;
}

uint32_t pmm_get_total_pages(void) {
    return total_pages;
}
