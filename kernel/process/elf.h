#ifndef _ELF_H
#define _ELF_H

#include "../types.h"

/* ELF file header */
#define EI_NIDENT 16

typedef struct {
    uint8_t  e_ident[EI_NIDENT];  /* Magic number and other info */
    uint16_t e_type;               /* Object file type */
    uint16_t e_machine;            /* Architecture */
    uint32_t e_version;            /* Object file version */
    uint64_t e_entry;              /* Entry point virtual address */
    uint64_t e_phoff;              /* Program header table file offset */
    uint64_t e_shoff;              /* Section header table file offset */
    uint32_t e_flags;              /* Processor-specific flags */
    uint16_t e_ehsize;             /* ELF header size in bytes */
    uint16_t e_phentsize;          /* Program header table entry size */
    uint16_t e_phnum;              /* Program header table entry count */
    uint16_t e_shentsize;          /* Section header table entry size */
    uint16_t e_shnum;              /* Section header table entry count */
    uint16_t e_shstrndx;           /* Section header string table index */
} Elf64_Ehdr;

/* Program segment header */
typedef struct {
    uint32_t p_type;               /* Segment type */
    uint32_t p_flags;              /* Segment flags */
    uint64_t p_offset;             /* Segment file offset */
    uint64_t p_vaddr;              /* Segment virtual address */
    uint64_t p_paddr;              /* Segment physical address */
    uint64_t p_filesz;             /* Segment size in file */
    uint64_t p_memsz;              /* Segment size in memory */
    uint64_t p_align;              /* Segment alignment */
} Elf64_Phdr;

/* ELF identification */
#define EI_MAG0    0  /* File identification */
#define EI_MAG1    1
#define EI_MAG2    2
#define EI_MAG3    3
#define EI_CLASS   4  /* File class */
#define EI_DATA    5  /* Data encoding */
#define EI_VERSION 6  /* File version */

/* Magic numbers */
#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

/* File class */
#define ELFCLASS64 2  /* 64-bit objects */

/* Data encoding */
#define ELFDATA2LSB 1  /* Little-endian */

/* Object file types */
#define ET_EXEC 2      /* Executable file */

/* Machine types */
#define EM_RISCV 243   /* RISC-V */

/* Program header types */
#define PT_NULL    0   /* Unused */
#define PT_LOAD    1   /* Loadable segment */
#define PT_DYNAMIC 2   /* Dynamic linking information */
#define PT_INTERP  3   /* Interpreter information */
#define PT_NOTE    4   /* Auxiliary information */

/* Program header flags */
#define PF_X 0x1       /* Executable */
#define PF_W 0x2       /* Writable */
#define PF_R 0x4       /* Readable */

/* ELF loader functions */
int elf_load(const uint8_t *binary, size_t size, uint64_t *entry);
int elf_validate(const Elf64_Ehdr *ehdr);

#endif /* _ELF_H */
