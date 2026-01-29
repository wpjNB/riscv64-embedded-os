/* RISC-V 64-bit Kernel Linker Script */

OUTPUT_ARCH(riscv)
ENTRY(_start)

/* Memory layout for QEMU virt machine */
MEMORY {
    RAM (rwx) : ORIGIN = 0x80200000, LENGTH = 126M
}

SECTIONS {
    . = 0x80200000;

    /* Boot section */
    .text.boot : {
        KEEP(*(.text.boot))
    } > RAM

    /* Code section */
    .text : {
        *(.text .text.*)
    } > RAM

    /* Read-only data */
    .rodata : ALIGN(8) {
        *(.rodata .rodata.*)
        *(.srodata .srodata.*)
    } > RAM

    /* Data section */
    .data : ALIGN(8) {
        __global_pointer$ = . + 0x800;
        *(.data .data.*)
        *(.sdata .sdata.*)
    } > RAM

    /* BSS section */
    .bss : ALIGN(8) {
        __bss_start = .;
        *(.bss .bss.*)
        *(.sbss .sbss.*)
        *(COMMON)
        __bss_end = .;
    } > RAM

    /* Stack section */
    . = ALIGN(16);
    __stack_start = .;
    . += 0x10000; /* 64KB stack */
    __stack_top = .;

    /* Heap section */
    . = ALIGN(4096);
    __heap_start = .;
    . += 0x400000; /* 4MB heap */
    __heap_end = .;

    /* Kernel end */
    . = ALIGN(4096);
    __kernel_end = .;

    /* Discard debug info */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}
