/* RISC-V Context Switch */

.section .text
.global switch_context

/*
 * switch_context(context_t *old, context_t *new)
 * a0: pointer to old context
 * a1: pointer to new context
 * 
 * Save current context to old, restore context from new
 */
switch_context:
    # Save old context (if old is not NULL)
    beqz a0, restore_new  # Skip save if old == NULL
    
    # Save callee-saved registers to old context
    sd ra, 0(a0)      # Save return address
    sd sp, 8(a0)      # Save stack pointer
    sd s0, 16(a0)     # Save s0
    sd s1, 24(a0)     # Save s1
    sd s2, 32(a0)     # Save s2
    sd s3, 40(a0)     # Save s3
    sd s4, 48(a0)     # Save s4
    sd s5, 56(a0)     # Save s5
    sd s6, 64(a0)     # Save s6
    sd s7, 72(a0)     # Save s7
    sd s8, 80(a0)     # Save s8
    sd s9, 88(a0)     # Save s9
    sd s10, 96(a0)    # Save s10
    sd s11, 104(a0)   # Save s11

restore_new:
    # Restore new context
    ld ra, 0(a1)      # Restore return address
    ld sp, 8(a1)      # Restore stack pointer
    ld s0, 16(a1)     # Restore s0
    ld s1, 24(a1)     # Restore s1
    ld s2, 32(a1)     # Restore s2
    ld s3, 40(a1)     # Restore s3
    ld s4, 48(a1)     # Restore s4
    ld s5, 56(a1)     # Restore s5
    ld s6, 64(a1)     # Restore s6
    ld s7, 72(a1)     # Restore s7
    ld s8, 80(a1)     # Restore s8
    ld s9, 88(a1)     # Restore s9
    ld s10, 96(a1)    # Restore s10
    ld s11, 104(a1)   # Restore s11
    
    # Return to new context (ra now points to new process)
    ret
