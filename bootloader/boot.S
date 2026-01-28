/* RISC-V 64-bit Boot Entry Point */

.section .text.boot
.global _start

_start:
    # Disable interrupts
    csrw sie, zero
    csrw sip, zero

    # Setup stack pointer
    la sp, __stack_top

    # Clear BSS section
    la t0, __bss_start
    la t1, __bss_end
clear_bss:
    beq t0, t1, clear_bss_done
    sd zero, 0(t0)
    addi t0, t0, 8
    j clear_bss

clear_bss_done:
    # Jump to kernel main
    call kernel_main

    # If kernel_main returns, halt
halt:
    wfi
    j halt

.section .text

# Trap handler entry (will be implemented later)
.global trap_entry
.align 4
trap_entry:
    # Save context
    csrw sscratch, sp
    
    # Call trap handler
    call trap_handler
    
    # Restore context and return
    csrr sp, sscratch
    sret
