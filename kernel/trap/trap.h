#ifndef _TRAP_H
#define _TRAP_H

#include "../types.h"

/* Trap initialization */
void trap_init(void);

/* Trap handler (called from assembly) */
void trap_handler(void);

#endif /* _TRAP_H */
