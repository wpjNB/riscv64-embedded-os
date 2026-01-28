#include "trap.h"
#include "../riscv.h"
#include "../printf.h"
#include "../process/scheduler.h"

extern void trap_entry(void);

void trap_init(void) {
    printf("[TRAP] Initializing trap handling\n");
    
    /* Set trap vector */
    w_stvec((uint64_t)trap_entry);
    
    /* Enable interrupts */
    w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);
    w_sstatus(r_sstatus() | SSTATUS_SIE);
    
    printf("[TRAP] Trap vector set to %p\n", (void*)r_stvec());
}

void trap_handler(void) {
    uint64_t scause = r_scause();
    uint64_t sepc = r_sepc();
    uint64_t stval = r_stval();
    
    /* Check if it's an interrupt or exception */
    if (scause & INTERRUPT_BIT) {
        /* Interrupt */
        uint64_t int_num = scause & ~INTERRUPT_BIT;
        
        switch (int_num) {
            case 1: /* Supervisor software interrupt */
                printf("[TRAP] Software interrupt\n");
                break;
            case 5: /* Supervisor timer interrupt */
                /* Timer interrupt - call scheduler for preemption */
                sched_tick();
                break;
            case 9: /* Supervisor external interrupt */
                printf("[TRAP] External interrupt\n");
                break;
            default:
                printf("[TRAP] Unknown interrupt: %u\n", (uint32_t)int_num);
                break;
        }
    } else {
        /* Exception */
        printf("\n[TRAP] Exception occurred!\n");
        printf("  scause: %p\n", (void*)scause);
        printf("  sepc:   %p\n", (void*)sepc);
        printf("  stval:  %p\n", (void*)stval);
        
        switch (scause) {
            case CAUSE_MISALIGNED_FETCH:
                printf("  Instruction address misaligned\n");
                break;
            case CAUSE_FETCH_ACCESS:
                printf("  Instruction access fault\n");
                break;
            case CAUSE_ILLEGAL_INSTRUCTION:
                printf("  Illegal instruction\n");
                break;
            case CAUSE_BREAKPOINT:
                printf("  Breakpoint\n");
                break;
            case CAUSE_MISALIGNED_LOAD:
                printf("  Load address misaligned\n");
                break;
            case CAUSE_LOAD_ACCESS:
                printf("  Load access fault\n");
                break;
            case CAUSE_MISALIGNED_STORE:
                printf("  Store address misaligned\n");
                break;
            case CAUSE_STORE_ACCESS:
                printf("  Store access fault\n");
                break;
            case CAUSE_USER_ECALL:
                printf("  Environment call from U-mode\n");
                break;
            case CAUSE_SUPERVISOR_ECALL:
                printf("  Environment call from S-mode\n");
                break;
            default:
                printf("  Unknown exception: %u\n", (uint32_t)scause);
                break;
        }
        
        panic("Unhandled exception");
    }
}
