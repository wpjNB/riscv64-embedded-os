#include "vm.h"
#include "mm.h"
#include "../printf.h"
#include "../riscv.h"

pagetable_t kernel_pagetable;

/* Initialize virtual memory system */
void vm_init(void) {
    printf("[VM] Initializing SV39 virtual memory\n");
    kvminit();
}

/* Create the kernel's page table */
void kvminit(void) {
    kernel_pagetable = (pagetable_t)alloc_page();
    
    if (kernel_pagetable == NULL) {
        panic("kvminit: alloc_page failed");
    }
    
    printf("[VM] Created kernel page table at %p\n", kernel_pagetable);
    
    /* Map kernel text and data (identity mapping) */
    /* QEMU virt machine: kernel loaded at 0x80000000 */
    if (mappages(kernel_pagetable, KERNBASE, PHYSTOP - KERNBASE, 
                 KERNBASE, PTE_R | PTE_W | PTE_X) != 0) {
        panic("kvminit: mappages failed for kernel");
    }
    
    /* Map UART (0x10000000) */
    if (mappages(kernel_pagetable, 0x10000000, PGSIZE,
                 0x10000000, PTE_R | PTE_W) != 0) {
        panic("kvminit: mappages failed for UART");
    }
    
    /* Map PLIC (0x0C000000 - 0x10000000) */
    if (mappages(kernel_pagetable, 0x0C000000, 0x4000000,
                 0x0C000000, PTE_R | PTE_W) != 0) {
        panic("kvminit: mappages failed for PLIC");
    }
    
    /* Map CLINT (0x02000000) */
    if (mappages(kernel_pagetable, 0x02000000, 0x10000,
                 0x02000000, PTE_R | PTE_W) != 0) {
        panic("kvminit: mappages failed for CLINT");
    }
    
    printf("[VM] Kernel page table initialized\n");
}

/* Switch to using the kernel page table */
void kvminithart(void) {
    /* Make a SATP value from the kernel page table */
    uint64_t satp = SATP_SV39 | ((uint64_t)kernel_pagetable >> 12);
    w_satp(satp);
    sfence_vma();
    
    printf("[VM] Switched to SV39 paging mode\n");
}

/* Walk the page table to find the PTE for a virtual address */
pte_t *walk(pagetable_t pagetable, uint64_t va, int alloc) {
    if (va >= MAXVA) {
        return NULL;
    }
    
    for (int level = 2; level > 0; level--) {
        pte_t *pte = &pagetable[PX(level, va)];
        
        if (PTE_VALID(*pte)) {
            pagetable = (pagetable_t)PTE2PA(*pte);
        } else {
            if (!alloc) {
                return NULL;
            }
            
            /* Allocate a new page table page */
            pagetable = (pagetable_t)alloc_page();
            if (pagetable == NULL) {
                return NULL;
            }
            
            /* Update the PTE to point to the new page */
            *pte = PA2PTE(pagetable) | PTE_V;
        }
    }
    
    return &pagetable[PX(0, va)];
}

/* Create mappings in the page table for a range of virtual addresses */
int mappages(pagetable_t pagetable, uint64_t va, uint64_t size, uint64_t pa, int perm) {
    uint64_t a, last;
    pte_t *pte;
    
    if (size == 0) {
        panic("mappages: size is zero");
    }
    
    a = (va / PGSIZE) * PGSIZE;  /* Round down to page boundary */
    last = ((va + size - 1) / PGSIZE) * PGSIZE;
    
    for (;;) {
        pte = walk(pagetable, a, 1);
        if (pte == NULL) {
            return -1;
        }
        
        if (PTE_VALID(*pte)) {
            panic("mappages: remap");
        }
        
        *pte = PA2PTE(pa) | perm | PTE_V;
        
        if (a == last) {
            break;
        }
        
        a += PGSIZE;
        pa += PGSIZE;
    }
    
    return 0;
}

/* Remove mappings from the page table */
void unmappages(pagetable_t pagetable, uint64_t va, uint64_t size) {
    uint64_t a, last;
    pte_t *pte;
    
    if (size == 0) {
        return;
    }
    
    a = (va / PGSIZE) * PGSIZE;
    last = ((va + size - 1) / PGSIZE) * PGSIZE;
    
    for (;;) {
        pte = walk(pagetable, a, 0);
        if (pte == NULL || !PTE_VALID(*pte)) {
            panic("unmappages: not mapped");
        }
        
        *pte = 0;
        
        if (a == last) {
            break;
        }
        
        a += PGSIZE;
    }
}

/* Look up a virtual address, return the physical address */
uint64_t walkaddr(pagetable_t pagetable, uint64_t va) {
    pte_t *pte;
    uint64_t pa;
    
    if (va >= MAXVA) {
        return 0;
    }
    
    pte = walk(pagetable, va, 0);
    if (pte == NULL || !PTE_VALID(*pte)) {
        return 0;
    }
    
    pa = PTE2PA(*pte);
    return pa;
}

/* Translate physical address to virtual (identity mapping for kernel) */
void *pa2va(uint64_t pa) {
    return (void*)pa;
}

/* Translate virtual address to physical using page table */
uint64_t va2pa(pagetable_t pagetable, uint64_t va) {
    pte_t *pte;
    uint64_t pa;
    
    pte = walk(pagetable, va, 0);
    if (pte == NULL || !PTE_VALID(*pte)) {
        return 0;
    }
    
    pa = PTE2PA(*pte);
    return pa + (va & (PGSIZE - 1));
}

/* Create a new empty page table for user space */
pagetable_t vm_create_user_pagetable(void) {
    pagetable_t pagetable = (pagetable_t)alloc_page();
    if (pagetable == NULL) {
        return NULL;
    }
    
    return pagetable;
}

/* Free a page table and all its pages */
void vm_free(pagetable_t pagetable) {
    /* Walk through all PTEs and free allocated pages */
    for (int i = 0; i < 512; i++) {
        pte_t pte = pagetable[i];
        if (PTE_VALID(pte) && !(pte & (PTE_R | PTE_W | PTE_X))) {
            /* This PTE points to a lower-level page table */
            uint64_t child = PTE2PA(pte);
            vm_free((pagetable_t)child);
        }
    }
    
    free_page(pagetable);
}

/* Create kernel page table (exported function) */
pagetable_t vm_create_kernel_pagetable(void) {
    return kernel_pagetable;
}
