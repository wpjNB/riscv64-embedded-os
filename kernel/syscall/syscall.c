#include "syscall.h"
#include "../printf.h"
#include "../../drivers/uart/uart.h"

void syscall_init(void) {
    /* Nothing to initialize for now */
}

uint64_t syscall_handler(uint64_t num, uint64_t arg0, uint64_t arg1, uint64_t arg2) {
    (void)arg2; /* Unused for now */
    
    switch (num) {
        case SYS_READ: {
            /* Read from stdin */
            char *buf = (char*)arg0;
            size_t len = (size_t)arg1;
            for (size_t i = 0; i < len; i++) {
                buf[i] = uart_getc();
                if (buf[i] == '\n') {
                    return i + 1;
                }
            }
            return len;
        }
        
        case SYS_WRITE: {
            /* Write to stdout */
            const char *buf = (const char*)arg0;
            size_t len = (size_t)arg1;
            for (size_t i = 0; i < len; i++) {
                uart_putc(buf[i]);
            }
            return len;
        }
        
        case SYS_FORK: {
            /* Fork not implemented yet */
            printf("[SYSCALL] fork() not implemented\n");
            return -1;
        }
        
        case SYS_EXEC: {
            /* Exec not implemented yet */
            printf("[SYSCALL] exec() not implemented\n");
            return -1;
        }
        
        case SYS_EXIT: {
            /* Exit current process */
            printf("[SYSCALL] Process exit with code %u\n", (uint32_t)arg0);
            return 0;
        }
        
        default:
            printf("[SYSCALL] Unknown syscall: %u\n", (uint32_t)num);
            return -1;
    }
}
