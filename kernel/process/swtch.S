/* Context switch assembly code for RISC-V 64-bit */
/* 
 * void swtch(context_t *old, context_t *new)
 * 
 * Saves the current process context to 'old' and restores 
 * the context from 'new'. This is a low-level context switch
 * that saves/restores only callee-saved registers.
 */

.section .text
.global swtch
.align 4

swtch:
    # a0 = old context pointer
    # a1 = new context pointer
    
    # Save old context (callee-saved registers)
    # If old context is NULL, skip saving
    beqz a0, restore_new
    
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
    # If new context is NULL, just return
    beqz a1, done
    
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

done:
    ret               # Return to new context


/*
 * void switch_to_user(uint64_t satp, uint64_t pc, uint64_t sp)
 * 
 * Switches from kernel mode to user mode and starts executing
 * user code at the specified PC with the given stack pointer.
 * 
 * a0 = satp value (page table)
 * a1 = user program counter
 * a2 = user stack pointer
 */
.global switch_to_user
.align 4

switch_to_user:
    # Set up SATP for user page table
    csrw satp, a0
    sfence.vma
    
    # Set up user stack pointer in sscratch
    csrw sscratch, a2
    
    # Set sepc to user program counter
    csrw sepc, a1
    
    # Set up sstatus for user mode
    # Clear SPP bit (previous mode = user)
    # Set SPIE bit (enable interrupts in user mode)
    li t0, 0x20      # SPIE bit
    csrw sstatus, t0
    
    # Return to user mode via sret
    # This will:
    # - Jump to sepc (user PC)
    # - Change to user mode (SPP bit was cleared)
    # - Enable interrupts (SPIE -> SIE)
    sret


/*
 * Trap entry point for user mode traps
 * This is called when user code traps into kernel
 */
.global trap_vector_user
.align 4

trap_vector_user:
    # Swap sp with sscratch to get kernel stack
    csrrw sp, sscratch, sp
    
    # Now sp points to kernel stack
    # sscratch contains user stack pointer
    
    # Save all registers to kernel stack
    addi sp, sp, -256   # Make space for trap frame
    
    # Save user registers
    sd x1, 0(sp)        # ra
    sd x2, 8(sp)        # sp (will be fixed later)
    sd x3, 16(sp)       # gp
    sd x4, 24(sp)       # tp
    sd x5, 32(sp)       # t0
    sd x6, 40(sp)       # t1
    sd x7, 48(sp)       # t2
    sd x8, 56(sp)       # s0/fp
    sd x9, 64(sp)       # s1
    sd x10, 72(sp)      # a0
    sd x11, 80(sp)      # a1
    sd x12, 88(sp)      # a2
    sd x13, 96(sp)      # a3
    sd x14, 104(sp)     # a4
    sd x15, 112(sp)     # a5
    sd x16, 120(sp)     # a6
    sd x17, 128(sp)     # a7
    sd x18, 136(sp)     # s2
    sd x19, 144(sp)     # s3
    sd x20, 152(sp)     # s4
    sd x21, 160(sp)     # s5
    sd x22, 168(sp)     # s6
    sd x23, 176(sp)     # s7
    sd x24, 184(sp)     # s8
    sd x25, 192(sp)     # s9
    sd x26, 200(sp)     # s10
    sd x27, 208(sp)     # s11
    sd x28, 216(sp)     # t3
    sd x29, 224(sp)     # t4
    sd x30, 232(sp)     # t5
    sd x31, 240(sp)     # t6
    
    # Save user stack pointer from sscratch
    csrr t0, sscratch
    sd t0, 8(sp)        # Save user sp
    
    # Call trap handler in C
    mv a0, sp           # Pass trap frame as argument
    call trap_handler
    
    # Restore user registers
    ld x1, 0(sp)
    ld x3, 16(sp)
    ld x4, 24(sp)
    ld x5, 32(sp)
    ld x6, 40(sp)
    ld x7, 48(sp)
    ld x8, 56(sp)
    ld x9, 64(sp)
    ld x10, 72(sp)
    ld x11, 80(sp)
    ld x12, 88(sp)
    ld x13, 96(sp)
    ld x14, 104(sp)
    ld x15, 112(sp)
    ld x16, 120(sp)
    ld x17, 128(sp)
    ld x18, 136(sp)
    ld x19, 144(sp)
    ld x20, 152(sp)
    ld x21, 160(sp)
    ld x22, 168(sp)
    ld x23, 176(sp)
    ld x24, 184(sp)
    ld x25, 192(sp)
    ld x26, 200(sp)
    ld x27, 208(sp)
    ld x28, 216(sp)
    ld x29, 224(sp)
    ld x30, 232(sp)
    ld x31, 240(sp)
    
    # Restore user sp
    ld x2, 8(sp)
    
    # Restore kernel stack
    addi sp, sp, 256
    
    # Swap back to user stack
    csrrw sp, sscratch, sp
    
    # Return to user mode
    sret
