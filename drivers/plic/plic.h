#ifndef _PLIC_H
#define _PLIC_H

#include "../types.h"

/* PLIC initialization */
void plic_init(void);

/* Enable/disable interrupt */
void plic_enable(uint32_t irq);
void plic_disable(uint32_t irq);

/* Claim and complete interrupt */
uint32_t plic_claim(void);
void plic_complete(uint32_t irq);

#endif /* _PLIC_H */
