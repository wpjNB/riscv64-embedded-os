#ifndef _MM_H
#define _MM_H

#include "../types.h"

#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

/* Memory management initialization */
void mm_init(void);

/* Page allocation */
void* alloc_page(void);
void free_page(void* page);

/* Simple heap allocator */
void* kmalloc(size_t size);
void kfree(void* ptr);

#endif /* _MM_H */
