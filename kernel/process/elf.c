#include "elf.h"
#include "../printf.h"
#include "../mm/mm.h"
#include "../mm/vm.h"

/* Validate ELF header */
int elf_validate(const Elf64_Ehdr *ehdr) {
    /* Check magic number */
    if (ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
        ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
        ehdr->e_ident[EI_MAG3] != ELFMAG3) {
        printf("[ELF] Invalid magic number\n");
        return -1;
    }
    
    /* Check class (64-bit) */
    if (ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
        printf("[ELF] Not a 64-bit ELF\n");
        return -1;
    }
    
    /* Check data encoding (little-endian) */
    if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
        printf("[ELF] Not little-endian\n");
        return -1;
    }
    
    /* Check machine type (RISC-V) */
    if (ehdr->e_machine != EM_RISCV) {
        printf("[ELF] Not a RISC-V binary\n");
        return -1;
    }
    
    /* Check file type (executable) */
    if (ehdr->e_type != ET_EXEC) {
        printf("[ELF] Not an executable\n");
        return -1;
    }
    
    return 0;
}

/* Load ELF binary into memory */
int elf_load(const uint8_t *binary, size_t size, uint64_t *entry) {
    const Elf64_Ehdr *ehdr = (const Elf64_Ehdr *)binary;
    
    /* Validate header */
    if (elf_validate(ehdr) != 0) {
        return -1;
    }
    
    printf("[ELF] Loading ELF binary...\n");
    printf("[ELF] Entry point: %p\n", (void*)ehdr->e_entry);
    printf("[ELF] Program headers: %u\n", ehdr->e_phnum);
    
    /* Get program headers */
    const Elf64_Phdr *phdr = (const Elf64_Phdr *)(binary + ehdr->e_phoff);
    
    /* Load each loadable segment */
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD) {
            continue;
        }
        
        printf("[ELF] Segment %d: vaddr=%p filesz=%lu memsz=%lu flags=%x\n",
               i, (void*)phdr[i].p_vaddr, phdr[i].p_filesz, 
               phdr[i].p_memsz, phdr[i].p_flags);
        
        /* Calculate permissions */
        int perm = 0;
        if (phdr[i].p_flags & PF_R) perm |= PTE_R;
        if (phdr[i].p_flags & PF_W) perm |= PTE_W;
        if (phdr[i].p_flags & PF_X) perm |= PTE_X;
        perm |= PTE_U;  /* User accessible */
        
        /* For now, we just validate the structure */
        /* Full implementation would:
         * 1. Allocate physical pages
         * 2. Map them in user page table
         * 3. Copy segment data from binary
         * 4. Zero out BSS region (memsz > filesz)
         */
        
        if (phdr[i].p_vaddr + phdr[i].p_memsz > MAXVA) {
            printf("[ELF] Segment virtual address out of range\n");
            return -1;
        }
    }
    
    /* Return entry point */
    *entry = ehdr->e_entry;
    
    printf("[ELF] ELF loaded successfully\n");
    return 0;
}
