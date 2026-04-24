#include "kheap.h"
#include "vmm.h"
#include "pmm.h"
#include "types.h"
#include "stddef.h"
#include "string.h"

typedef struct block_header {
    uint32_t size;
    int is_free;
    struct block_header* next;
    struct block_header* prev;
} block_header_t;

static block_header_t* heap_start_block = NULL;
static uint32_t heap_end = KHEAP_START;
static uint32_t heap_used = 0;
static int heap_initialized = 0;

static uint32_t align_up(uint32_t value, uint32_t alignment) {
    if (alignment == 0) {
        return value;
    }

    return (value + alignment - 1) & ~(alignment - 1);
}

static block_header_t* heap_last_block(void) {
    block_header_t* current = heap_start_block;

    if (!current) {
        return NULL;
    }

    while (current->next) {
        current = current->next;
    }

    return current;
}

static void merge_with_next(block_header_t* block) {
    block_header_t* next;

    if (!block || !block->next || !block->next->is_free) {
        return;
    }

    next = block->next;
    block->size += sizeof(block_header_t) + next->size;
    block->next = next->next;

    if (block->next) {
        block->next->prev = block;
    }
}

static void split_block(block_header_t* block, uint32_t size) {
    uint32_t remaining;
    block_header_t* new_block;

    remaining = block->size - size;
    if (remaining <= (sizeof(block_header_t) + 16)) {
        return;
    }

    new_block = (block_header_t*)((uint32_t)block + sizeof(block_header_t) + size);
    new_block->size = remaining - sizeof(block_header_t);
    new_block->is_free = 1;
    new_block->next = block->next;
    new_block->prev = block;

    if (new_block->next) {
        new_block->next->prev = new_block;
    }

    block->size = size;
    block->next = new_block;
}

static int heap_map_page(uint32_t virtual_addr) {
    if (!current_directory) {
        return -1;
    }

    if (!vmm_alloc_page(current_directory, virtual_addr, PAGE_PRESENT | PAGE_WRITE)) {
        return -1;
    }

    memset((void*)virtual_addr, 0, PAGE_SIZE);
    return 0;
}

static block_header_t* heap_expand(uint32_t min_payload_size) {
    block_header_t* last;
    block_header_t* block;
    uint32_t old_end;
    uint32_t bytes_to_map;
    uint32_t target_end;

    old_end = heap_end;
    bytes_to_map = align_up(sizeof(block_header_t) + min_payload_size, PAGE_SIZE);
    target_end = old_end + bytes_to_map;

    if (target_end > KHEAP_END) {
        return NULL;
    }

    while (heap_end < target_end) {
        if (heap_map_page(heap_end) != 0) {
            return NULL;
        }

        heap_end += PAGE_SIZE;
    }

    last = heap_last_block();
    if (last && last->is_free) {
        last->size += heap_end - old_end;
        return last;
    }

    block = (block_header_t*)old_end;
    block->size = (heap_end - old_end) - sizeof(block_header_t);
    block->is_free = 1;
    block->next = NULL;
    block->prev = last;

    if (last) {
        last->next = block;
    } else {
        heap_start_block = block;
    }

    return block;
}

static block_header_t* find_free_block(uint32_t size) {
    block_header_t* current = heap_start_block;

    while (current) {
        if (current->is_free && current->size >= size) {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

void kheap_init(void) {
    heap_start_block = NULL;
    heap_end = KHEAP_START;
    heap_used = 0;
    heap_initialized = 0;

    if (!heap_expand(0)) {
        return;
    }

    heap_initialized = 1;
}

void* kmalloc(uint32_t size) {
    block_header_t* block;

    if (size == 0 || !heap_initialized) {
        return NULL;
    }

    size = align_up(size, 16);
    block = find_free_block(size);

    if (!block) {
        block = heap_expand(size);
        if (!block) {
            return NULL;
        }
    }

    split_block(block, size);
    block->is_free = 0;
    heap_used += block->size;

    return (void*)((uint32_t)block + sizeof(block_header_t));
}

void* kmalloc_align(uint32_t size, uint32_t alignment) {
    uint32_t aligned_addr;
    uint32_t raw_addr;
    void* raw_ptr;

    if (alignment == 0) {
        return kmalloc(size);
    }

    if ((alignment & (alignment - 1)) != 0) {
        return NULL;
    }

    raw_ptr = kmalloc(size + alignment + sizeof(void*));
    if (!raw_ptr) {
        return NULL;
    }

    raw_addr = (uint32_t)raw_ptr;
    aligned_addr = align_up(raw_addr + sizeof(void*), alignment);
    ((void**)aligned_addr)[-1] = raw_ptr;

    return (void*)aligned_addr;
}

void kfree(void* ptr) {
    block_header_t* block;

    if (!ptr || !heap_initialized) {
        return;
    }

    if ((uint32_t)ptr < (KHEAP_START + sizeof(block_header_t)) || (uint32_t)ptr >= heap_end) {
        return;
    }

    block = (block_header_t*)((uint32_t)ptr - sizeof(block_header_t));
    if (block->is_free) {
        return;
    }

    block->is_free = 1;
    if (heap_used >= block->size) {
        heap_used -= block->size;
    } else {
        heap_used = 0;
    }

    merge_with_next(block);
    if (block->prev && block->prev->is_free) {
        merge_with_next(block->prev);
    }
}

void kfree_align(void* ptr) {
    void* original_ptr;

    if (!ptr) {
        return;
    }

    original_ptr = ((void**)ptr)[-1];
    kfree(original_ptr);
}

int kheap_is_initialized(void) {
    return heap_initialized;
}

uint32_t kheap_get_used(void) {
    return heap_used;
}

uint32_t kheap_get_reserved(void) {
    return heap_end - KHEAP_START;
}

uint32_t kheap_get_limit(void) {
    return KHEAP_END - KHEAP_START;
}
