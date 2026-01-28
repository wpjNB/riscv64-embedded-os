#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "../types.h"

/* System call numbers */
#define SYS_READ  0
#define SYS_WRITE 1
#define SYS_FORK  2
#define SYS_EXEC  3
#define SYS_EXIT  4

/* System call initialization */
void syscall_init(void);

/* System call handler */
uint64_t syscall_handler(uint64_t num, uint64_t arg0, uint64_t arg1, uint64_t arg2);

#endif /* _SYSCALL_H */
