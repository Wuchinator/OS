#include "kheap.h"
#include "pmm.h"
#include "types.h"
#include "stddef.h"

#define HEAP_START 0x00400000
#define HEAP_END   0x04000000  

typedef struct block_header {
    uint32_t size;
    int is_free;
    struct block_header* next;
    struct block_header* original_block;
} block_header_t;

static block_header_t* heap_start_block = NULL;
static uint32_t heap_end = HEAP_START;
static uint32_t heap_used = 0;

static uint32_t align_up(uint32_t size, uint32_t alignment) {
    if (alignment == 0) return size;
    return (size + alignment - 1) & ~(alignment - 1);
}

static void heap_expand(uint32_t size) {
    uint32_t pages_needed = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_t i = 0; i < pages_needed; i++) {
        if (heap_end >= HEAP_END) {
            return;
        }

        
        heap_end += PAGE_SIZE;
    }
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

static block_header_t* create_block(uint32_t size) {
    block_header_t* block = (block_header_t*)heap_end;
    block->size = size;
    block->is_free = 0;
    block->next = NULL;
    block->original_block = block;
    heap_end += sizeof(block_header_t) + size;
    heap_used += size;
    return block;
}

static void split_block(block_header_t* block, uint32_t size) {
    uint32_t remaining = block->size - size;

    if (remaining > sizeof(block_header_t) + 64) {
        block_header_t* new_block = (block_header_t*)((uint32_t)block + sizeof(block_header_t) + size);
        new_block->size = remaining - sizeof(block_header_t);
        new_block->is_free = 1;
        new_block->next = block->next;

        block->size = size;
        block->next = new_block;
    }
}

void kheap_init(void) {
    heap_end = HEAP_START;
    heap_used = 0;
    heap_expand(PAGE_SIZE);
    heap_start_block = (block_header_t*)HEAP_START;
    heap_start_block->size = PAGE_SIZE - sizeof(block_header_t);
    heap_start_block->is_free = 1;
    heap_start_block->next = NULL;
    heap_start_block->original_block = heap_start_block;
}

void* kmalloc(uint32_t size) {
    if (size == 0) {
        return NULL;
    }
    size = align_up(size, 16);
    block_header_t* block = find_free_block(size);

    if (!block) {
        uint32_t expand_size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        heap_expand(expand_size);

        block = create_block(size);
    } else {
        block->is_free = 0;
        split_block(block, size);
        heap_used += size;
    }

    return (void*)((uint32_t)block + sizeof(block_header_t));
}

void* kmalloc_align(uint32_t size, uint32_t alignment) {
    uint32_t total_size = size + alignment;
    void* ptr = kmalloc(total_size);
    if (!ptr) {
        return NULL;
    }
    
    uint32_t addr = (uint32_t)ptr;
    uint32_t aligned_addr = align_up(addr, alignment);
    void* aligned_ptr = (void*)aligned_addr;    
    void** original_ptr_store = (void**)((uint32_t)aligned_ptr - sizeof(void*));
    *original_ptr_store = ptr;
    
    return aligned_ptr;
}

void kfree(void* ptr) {
    if (!ptr) {
        return;
    }

    block_header_t* block = (block_header_t*)((uint32_t)ptr - sizeof(block_header_t));
    block->is_free = 1;
    heap_used -= block->size;
    if (block->next && block->next->is_free) {
        block->size += sizeof(block_header_t) + block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->original_block = block->next;
        }
    }
        block_header_t* current = heap_start_block;
    block_header_t* prev = NULL;
    
    while (current && current != block) {
        prev = current;
        current = current->next;
    }
    
    if (prev && prev->is_free) {
        prev->size += sizeof(block_header_t) + block->size;
        prev->next = block->next;
        if (prev->next) {
            prev->next->original_block = prev->next;
        }
    }
}

void kfree_align(void* ptr) {
    if (!ptr) {
        return;
    }
    
    void** original_ptr_store = (void**)((uint32_t)ptr - sizeof(void*));
    void* original_ptr = *original_ptr_store;
    
    kfree(original_ptr);
}

uint32_t kheap_get_used(void) {
    return heap_used;
}
