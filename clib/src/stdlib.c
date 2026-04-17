#include "stdlib.h"
#include "string.h"


#define HEAP_SIZE 0x10000 

typedef struct Block {
    size_t size;
    int is_free;
    struct Block* next;
    struct Block* prev;
} Block;

static char heap[HEAP_SIZE] __attribute__((aligned(16)));
static Block* heap_start = NULL;
static int heap_initialized = 0;

static void init_heap(void) {
    if (heap_initialized) return;
    
    heap_start = (Block*)heap;
    heap_start->size = HEAP_SIZE - sizeof(Block);
    heap_start->is_free = 1;
    heap_start->next = NULL;
    heap_start->prev = NULL;
    
    heap_initialized = 1;
}

void* malloc(size_t size) {
    if (size == 0) return NULL;
    
    init_heap();
    
    
    size = (size + 15) & ~15;
    
    
    Block* current = heap_start;
    while (current) {
        if (current->is_free && current->size >= size) {
            
            
            
            size_t remaining = current->size - size - sizeof(Block);
            if (remaining > sizeof(Block) + 16) {
                Block* new_block = (Block*)((char*)current + sizeof(Block) + size);
                new_block->size = remaining;
                new_block->is_free = 1;
                new_block->next = current->next;
                new_block->prev = current;
                
                if (current->next) {
                    current->next->prev = new_block;
                }
                
                current->next = new_block;
                current->size = size;
            }
            
            current->is_free = 0;
            return (void*)((char*)current + sizeof(Block));
        }
        current = current->next;
    }
    
    
    return NULL;
}

void* calloc(size_t num, size_t size) {
    size_t total_size = num * size;
    void* ptr = malloc(total_size);
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

void* realloc(void* ptr, size_t new_size) {
    if (!ptr) return malloc(new_size);
    if (new_size == 0) {
        free(ptr);
        return NULL;
    }
    
    
    Block* block = (Block*)((char*)ptr - sizeof(Block));
    size_t old_size = block->size;
    
    if (new_size <= old_size) {
        return ptr; 
    }
    
    
    void* new_ptr = malloc(new_size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, old_size);
        free(ptr);
    }
    
    return new_ptr;
}

void free(void* ptr) {
    if (!ptr) return;
    
    Block* block = (Block*)((char*)ptr - sizeof(Block));
    block->is_free = 1;
    
    
    if (block->next && block->next->is_free) {
        block->size += sizeof(Block) + block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }
    
    
    if (block->prev && block->prev->is_free) {
        block->prev->size += sizeof(Block) + block->size;
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
    }
}


int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    
    
    while (*str == ' ' || *str == '\t') {
        str++;
    }
    
    
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result * sign;
}

long atol(const char* str) {
    long result = 0;
    int sign = 1;
    
    while (*str == ' ' || *str == '\t') {
        str++;
    }
    
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result * sign;
}

long long atoll(const char* str) {
    long long result = 0;
    int sign = 1;
    
    while (*str == ' ' || *str == '\t') {
        str++;
    }
    
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result * sign;
}

double atof(const char* str) {
    double result = 0.0;
    double fraction = 0.1;
    int sign = 1;
    
    while (*str == ' ' || *str == '\t') {
        str++;
    }
    
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    
    while (*str >= '0' && *str <= '9') {
        result = result * 10.0 + (*str - '0');
        str++;
    }
    
    
    if (*str == '.') {
        str++;
        while (*str >= '0' && *str <= '9') {
            result += (*str - '0') * fraction;
            fraction *= 0.1;
            str++;
        }
    }
    
    return result * sign;
}

long strtol(const char* str, char** endptr, int base) {
    long result = 0;
    int sign = 1;
    
    while (*str == ' ' || *str == '\t') {
        str++;
    }
    
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    
    if (base == 0 || base == 16) {
        if (*str == '0' && (*(str + 1) == 'x' || *(str + 1) == 'X')) {
            base = 16;
            str += 2;
        } else if (base == 0) {
            base = 10;
        }
    }
    
    if (base == 0) {
        if (*str == '0') {
            base = 8;
        } else {
            base = 10;
        }
    }
    
    while (*str) {
        int digit = -1;
        if (*str >= '0' && *str <= '9') {
            digit = *str - '0';
        } else if (*str >= 'a' && *str <= 'f') {
            digit = *str - 'a' + 10;
        } else if (*str >= 'A' && *str <= 'F') {
            digit = *str - 'A' + 10;
        }
        
        if (digit >= 0 && digit < base) {
            result = result * base + digit;
            str++;
        } else {
            break;
        }
    }
    
    if (endptr) {
        *endptr = (char*)str;
    }
    
    return result * sign;
}

unsigned long strtoul(const char* str, char** endptr, int base) {
    unsigned long result = 0;
    
    while (*str == ' ' || *str == '\t') {
        str++;
    }
    
    if (*str == '+') {
        str++;
    }
    
    
    if (base == 0 || base == 16) {
        if (*str == '0' && (*(str + 1) == 'x' || *(str + 1) == 'X')) {
            base = 16;
            str += 2;
        } else if (base == 0) {
            base = 10;
        }
    }
    
    if (base == 0) {
        if (*str == '0') {
            base = 8;
        } else {
            base = 10;
        }
    }
    
    while (*str) {
        int digit = -1;
        if (*str >= '0' && *str <= '9') {
            digit = *str - '0';
        } else if (*str >= 'a' && *str <= 'f') {
            digit = *str - 'a' + 10;
        } else if (*str >= 'A' && *str <= 'F') {
            digit = *str - 'A' + 10;
        }
        
        if (digit >= 0 && digit < base) {
            result = result * base + digit;
            str++;
        } else {
            break;
        }
    }
    
    if (endptr) {
        *endptr = (char*)str;
    }
    
    return result;
}


int abs(int j) {
    return (j < 0) ? -j : j;
}

long labs(long j) {
    return (j < 0) ? -j : j;
}

long long llabs(long long j) {
    return (j < 0) ? -j : j;
}


static unsigned long rand_state = 1;

int rand(void) {
    rand_state = rand_state * 1103515245 + 12345;
    return (int)((unsigned long)(rand_state / 65536) % 32768);
}

void srand(unsigned int seed) {
    rand_state = seed;
}


void exit(int status) {
    
    
    (void)status;
    while (1) {
        
    }
}

int atexit(void (*function)(void)) {
    
    (void)function;
    return -1;
}

void abort(void) {
    
    while (1) {
        
    }
}


void* bsearch(const void* key, const void* base, size_t num, size_t size,
              int (*compar)(const void*, const void*)) {
    size_t left = 0;
    size_t right = num;
    
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        const void* mid_elem = (const char*)base + mid * size;
        int cmp = compar(key, mid_elem);
        
        if (cmp == 0) {
            return (void*)mid_elem;
        } else if (cmp < 0) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }
    
    return NULL;
}


static void swap(char* a, char* b, size_t size) {
    for (size_t i = 0; i < size; i++) {
        char temp = a[i];
        a[i] = b[i];
        b[i] = temp;
    }
}

static void qsort_recursive(void* base, size_t num, size_t size,
                            int (*compar)(const void*, const void*)) {
    if (num <= 1) return;
    
    
    size_t pivot_idx = num / 2;
    char* pivot = (char*)base + pivot_idx * size;
    
    
    size_t i = 0;
    size_t j = num - 1;
    
    while (i <= j) {
        while (compar((char*)base + i * size, pivot) < 0) i++;
        while (compar((char*)base + j * size, pivot) > 0) j--;
        
        if (i <= j) {
            swap((char*)base + i * size, (char*)base + j * size, size);
            i++;
            if (j > 0) j--;
        }
    }
    
    
    if (j > 0) qsort_recursive(base, j + 1, size, compar);
    if (i < num) qsort_recursive((char*)base + i * size, num - i, size, compar);
}

void qsort(void* base, size_t num, size_t size,
           int (*compar)(const void*, const void*)) {
    qsort_recursive(base, num, size, compar);
}
