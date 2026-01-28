#ifndef _VM_H
#define _VM_H

#include "../types.h"

/* SV39 Virtual Memory Configuration */
#define SATP_SV39 (8UL << 60)
#define PGSIZE 4096
#define PGSHIFT 12

/* Virtual address space layout */
#define MAXVA (1UL << (9 + 9 + 9 + 12 - 1))  /* 256GB (half of 512GB space) */
#define KERNBASE 0x80000000UL
#define PHYSTOP (KERNBASE + 128*1024*1024)  /* 128MB physical memory */

/* Page table entry (PTE) fields */
#define PTE_V    (1UL << 0)  /* Valid */
#define PTE_R    (1UL << 1)  /* Readable */
#define PTE_W    (1UL << 2)  /* Writable */
#define PTE_X    (1UL << 3)  /* Executable */
#define PTE_U    (1UL << 4)  /* User accessible */
#define PTE_G    (1UL << 5)  /* Global */
#define PTE_A    (1UL << 6)  /* Accessed */
#define PTE_D    (1UL << 7)  /* Dirty */

/* Page table entry flags */
#define PTE_FLAGS(pte) ((pte) & 0x3FF)
#define PA2PTE(pa) ((((uint64_t)pa) >> 12) << 10)
#define PTE2PA(pte) (((pte) >> 10) << 12)
#define PTE_VALID(pte) ((pte) & PTE_V)

/* Extract the three 9-bit page table indices from a virtual address */
#define PXMASK 0x1FF  /* 9 bits */
#define PXSHIFT(level) (PGSHIFT + (9 * (level)))
#define PX(level, va) ((((uint64_t)(va)) >> PXSHIFT(level)) & PXMASK)

/* Page table structure */
typedef uint64_t pte_t;
typedef uint64_t *pagetable_t;  /* 512 PTEs */

/* Virtual memory functions */
void vm_init(void);
pagetable_t vm_create_kernel_pagetable(void);
pagetable_t vm_create_user_pagetable(void);
void vm_free(pagetable_t pagetable);

/* Page table manipulation */
pte_t *walk(pagetable_t pagetable, uint64_t va, int alloc);
int mappages(pagetable_t pagetable, uint64_t va, uint64_t size, uint64_t pa, int perm);
void unmappages(pagetable_t pagetable, uint64_t va, uint64_t size);
uint64_t walkaddr(pagetable_t pagetable, uint64_t va);

/* Address translation */
void *pa2va(uint64_t pa);
uint64_t va2pa(pagetable_t pagetable, uint64_t va);

/* Kernel page table */
extern pagetable_t kernel_pagetable;

/* Initialize paging for the kernel */
void kvminit(void);
void kvminithart(void);

#endif /* _VM_H */
