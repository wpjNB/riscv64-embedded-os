/* RISC-V Context Switch */

.section .text
.global switch_context

/*
 * switch_context(context_t *old, context_t *new)
 * a0: pointer to old context
 * a1: pointer to new context
 * 
 * Complete context switch: Save current context to old, restore context from new
 * This saves all callee-saved registers as required by RISC-V calling convention
 */
switch_context:
    # Save old context (if old is not NULL)
    beqz a0, restore_new  # Skip save if old == NULL
    
    # Save all callee-saved registers to old context
    # According to RISC-V calling convention, we need to save:
    # - ra (return address)
    # - sp (stack pointer)
    # - s0-s11 (saved registers)
    
    sd ra, 0(a0)      # Offset 0: Save return address
    sd sp, 8(a0)      # Offset 8: Save stack pointer
    sd s0, 16(a0)     # Offset 16: Save s0 (fp)
    sd s1, 24(a0)     # Offset 24: Save s1
    sd s2, 32(a0)     # Offset 32: Save s2
    sd s3, 40(a0)     # Offset 40: Save s3
    sd s4, 48(a0)     # Offset 48: Save s4
    sd s5, 56(a0)     # Offset 56: Save s5
    sd s6, 64(a0)     # Offset 64: Save s6
    sd s7, 72(a0)     # Offset 72: Save s7
    sd s8, 80(a0)     # Offset 80: Save s8
    sd s9, 88(a0)     # Offset 88: Save s9
    sd s10, 96(a0)    # Offset 96: Save s10
    sd s11, 104(a0)   # Offset 104: Save s11

restore_new:
    # Restore new context from new process
    # Load all callee-saved registers from new context
    
    ld ra, 0(a1)      # Offset 0: Restore return address
    ld sp, 8(a1)      # Offset 8: Restore stack pointer
    ld s0, 16(a1)     # Offset 16: Restore s0 (fp)
    ld s1, 24(a1)     # Offset 24: Restore s1
    ld s2, 32(a1)     # Offset 32: Restore s2
    ld s3, 40(a1)     # Offset 40: Restore s3
    ld s4, 48(a1)     # Offset 48: Restore s4
    ld s5, 56(a1)     # Offset 56: Restore s5
    ld s6, 64(a1)     # Offset 64: Restore s6
    ld s7, 72(a1)     # Offset 72: Restore s7
    ld s8, 80(a1)     # Offset 80: Restore s8
    ld s9, 88(a1)     # Offset 88: Restore s9
    ld s10, 96(a1)    # Offset 96: Restore s10
    ld s11, 104(a1)   # Offset 104: Restore s11
    
    # Return to new context
    # The 'ret' instruction will jump to the address in ra,
    # which now points to the new process's continuation point
    ret

/*
 * Additional notes on context switching:
 * 
 * 1. Callee-saved registers (s0-s11, sp, ra) MUST be preserved across function calls
 * 2. Caller-saved registers (a0-a7, t0-t6) are preserved by the caller before calling
 * 3. When switching contexts, we only need to save/restore callee-saved registers
 * 4. The PC (program counter) is implicitly saved in 'ra' (return address)
 * 5. Upon 'ret', execution continues at the address stored in 'ra'
 * 
 * For trap/interrupt context switching (not implemented here):
 * - Would need to save ALL registers including caller-saved
 * - Would need to save/restore CSRs (sstatus, sepc, etc.)
 * - Would need to handle privilege mode transitions
 */
