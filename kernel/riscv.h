#ifndef _RISCV_H
#define _RISCV_H

#include "types.h"

/* RISC-V CSR registers */
#define CSR_SSTATUS  0x100
#define CSR_SIE      0x104
#define CSR_STVEC    0x105
#define CSR_SSCRATCH 0x140
#define CSR_SEPC     0x141
#define CSR_SCAUSE   0x142
#define CSR_STVAL    0x143
#define CSR_SIP      0x144
#define CSR_SATP     0x180

/* Status register bits */
#define SSTATUS_SIE  (1UL << 1)   /* Supervisor Interrupt Enable */
#define SSTATUS_SPIE (1UL << 5)   /* Previous Interrupt Enable */
#define SSTATUS_SPP  (1UL << 8)   /* Previous Privilege */

/* Interrupt bits */
#define SIE_SSIE     (1UL << 1)   /* Software interrupt */
#define SIE_STIE     (1UL << 5)   /* Timer interrupt */
#define SIE_SEIE     (1UL << 9)   /* External interrupt */

/* Trap causes */
#define CAUSE_MISALIGNED_FETCH    0
#define CAUSE_FETCH_ACCESS        1
#define CAUSE_ILLEGAL_INSTRUCTION 2
#define CAUSE_BREAKPOINT          3
#define CAUSE_MISALIGNED_LOAD     4
#define CAUSE_LOAD_ACCESS         5
#define CAUSE_MISALIGNED_STORE    6
#define CAUSE_STORE_ACCESS        7
#define CAUSE_USER_ECALL          8
#define CAUSE_SUPERVISOR_ECALL    9
#define CAUSE_HYPERVISOR_ECALL    10
#define CAUSE_MACHINE_ECALL       11

/* Interrupt bit */
#define INTERRUPT_BIT             (1UL << 63)

/* CSR read/write macros */
static inline uint64_t r_csr(uint64_t csr) {
    uint64_t val;
    asm volatile("csrr %0, %1" : "=r"(val) : "i"(csr));
    return val;
}

static inline void w_csr(uint64_t csr, uint64_t val) {
    asm volatile("csrw %0, %1" : : "i"(csr), "r"(val));
}

static inline uint64_t r_sstatus() {
    uint64_t x;
    asm volatile("csrr %0, sstatus" : "=r"(x));
    return x;
}

static inline void w_sstatus(uint64_t x) {
    asm volatile("csrw sstatus, %0" : : "r"(x));
}

static inline uint64_t r_sie() {
    uint64_t x;
    asm volatile("csrr %0, sie" : "=r"(x));
    return x;
}

static inline void w_sie(uint64_t x) {
    asm volatile("csrw sie, %0" : : "r"(x));
}

static inline uint64_t r_stvec() {
    uint64_t x;
    asm volatile("csrr %0, stvec" : "=r"(x));
    return x;
}

static inline void w_stvec(uint64_t x) {
    asm volatile("csrw stvec, %0" : : "r"(x));
}

static inline uint64_t r_sepc() {
    uint64_t x;
    asm volatile("csrr %0, sepc" : "=r"(x));
    return x;
}

static inline void w_sepc(uint64_t x) {
    asm volatile("csrw sepc, %0" : : "r"(x));
}

static inline uint64_t r_scause() {
    uint64_t x;
    asm volatile("csrr %0, scause" : "=r"(x));
    return x;
}

static inline uint64_t r_stval() {
    uint64_t x;
    asm volatile("csrr %0, stval" : "=r"(x));
    return x;
}

static inline uint64_t r_satp() {
    uint64_t x;
    asm volatile("csrr %0, satp" : "=r"(x));
    return x;
}

static inline void w_satp(uint64_t x) {
    asm volatile("csrw satp, %0" : : "r"(x));
}

/* Memory barrier */
static inline void sfence_vma() {
    asm volatile("sfence.vma zero, zero");
}

/* Wait for interrupt */
static inline void wfi() {
    asm volatile("wfi");
}

#endif /* _RISCV_H */
