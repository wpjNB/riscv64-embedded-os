#include "mm.h"
#include "../printf.h"

/* Defined in linker script */
extern char __heap_start[];
extern char __heap_end[];
extern char __kernel_end[];

/* Simple stack-based page allocator */
static uint64_t *free_pages = NULL;
static uint64_t num_free_pages = 0;
static void *heap_current = NULL;

void mm_init(void) {
    /* Initialize heap */
    heap_current = (void*)__heap_start;
    
    /* Calculate available memory after kernel */
    uint64_t mem_start = (uint64_t)__heap_end;
    uint64_t mem_end = 0x80000000 + (128 * 1024 * 1024); /* 128MB total RAM */
    
    /* Align to page boundary */
    mem_start = (mem_start + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    
    printf("[MM] Initializing memory manager\n");
    printf("[MM] Heap: %p - %p\n", __heap_start, __heap_end);
    printf("[MM] Free memory: %p - %p\n", (void*)mem_start, (void*)mem_end);
    
    /* Initialize free page list */
    free_pages = (uint64_t*)mem_start;
    num_free_pages = (mem_end - mem_start) / PAGE_SIZE;
    
    /* Build linked list of free pages */
    for (uint64_t i = 0; i < num_free_pages; i++) {
        uint64_t *page = (uint64_t*)(mem_start + i * PAGE_SIZE);
        if (i + 1 < num_free_pages) {
            *page = mem_start + (i + 1) * PAGE_SIZE;
        } else {
            *page = 0; /* End of list */
        }
    }
    
    printf("[MM] Initialized %u free pages (%u KB)\n", 
           (uint32_t)num_free_pages, (uint32_t)(num_free_pages * PAGE_SIZE / 1024));
}

void* alloc_page(void) {
    if (free_pages == NULL || num_free_pages == 0) {
        printf("[MM] Out of memory!\n");
        return NULL;
    }
    
    /* Pop from stack */
    void *page = (void*)free_pages;
    free_pages = (uint64_t*)(*free_pages);
    num_free_pages--;
    
    /* Clear page */
    uint64_t *p = (uint64_t*)page;
    for (int i = 0; i < PAGE_SIZE / 8; i++) {
        p[i] = 0;
    }
    
    return page;
}

void free_page(void* page) {
    if (page == NULL) {
        return;
    }
    
    /* Push to stack */
    uint64_t *p = (uint64_t*)page;
    *p = (uint64_t)free_pages;
    free_pages = p;
    num_free_pages++;
}

/* Simple bump allocator for small allocations */
void* kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    /* Align to 8 bytes */
    size = (size + 7) & ~7;
    
    /* Check if we have space */
    if ((uint64_t)heap_current + size > (uint64_t)__heap_end) {
        printf("[MM] Heap exhausted!\n");
        return NULL;
    }
    
    void *ptr = heap_current;
    heap_current = (void*)((uint64_t)heap_current + size);
    
    return ptr;
}

void kfree(void* ptr) {
    /* Simple bump allocator doesn't support free */
    (void)ptr;
}
