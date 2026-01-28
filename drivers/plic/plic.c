#include "plic.h"

#define PLIC_BASE 0x0C000000UL
#define PLIC_PRIORITY(id) (PLIC_BASE + (id) * 4)
#define PLIC_PENDING(id) (PLIC_BASE + 0x1000 + ((id) / 32) * 4)
#define PLIC_ENABLE(hart) (PLIC_BASE + 0x2000 + (hart) * 0x80)
#define PLIC_THRESHOLD(hart) (PLIC_BASE + 0x200000 + (hart) * 0x1000)
#define PLIC_CLAIM(hart) (PLIC_BASE + 0x200004 + (hart) * 0x1000)

void plic_init(void) {
    /* Set threshold to 0 (accept all interrupts) */
    volatile uint32_t *threshold = (volatile uint32_t*)PLIC_THRESHOLD(0);
    *threshold = 0;
}

void plic_enable(uint32_t irq) {
    /* Set priority */
    volatile uint32_t *priority = (volatile uint32_t*)PLIC_PRIORITY(irq);
    *priority = 1;
    
    /* Enable interrupt */
    volatile uint32_t *enable = (volatile uint32_t*)PLIC_ENABLE(0);
    enable[irq / 32] |= (1 << (irq % 32));
}

void plic_disable(uint32_t irq) {
    volatile uint32_t *enable = (volatile uint32_t*)PLIC_ENABLE(0);
    enable[irq / 32] &= ~(1 << (irq % 32));
}

uint32_t plic_claim(void) {
    volatile uint32_t *claim = (volatile uint32_t*)PLIC_CLAIM(0);
    return *claim;
}

void plic_complete(uint32_t irq) {
    volatile uint32_t *claim = (volatile uint32_t*)PLIC_CLAIM(0);
    *claim = irq;
}
