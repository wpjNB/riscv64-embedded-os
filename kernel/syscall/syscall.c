#include "syscall.h"
#include "../printf.h"
#include "../process/scheduler.h"
#include "../fs/vfs.h"
#include "../../drivers/uart/uart.h"

#define SYSCALL_ERROR ((uint64_t)-1)  /* Error return value (UINT64_MAX) */

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
            return SYSCALL_ERROR;
        }
        
        case SYS_EXEC: {
            /* Exec not implemented yet */
            printf("[SYSCALL] exec() not implemented\n");
            return SYSCALL_ERROR;
        }
        
        case SYS_EXIT: {
            /* Exit current process */
            printf("[SYSCALL] Process exit with code %u\n", (uint32_t)arg0);
            return 0;
        }
        
        case SYS_OPEN: {
            /* Open file */
            const char *path = (const char*)arg0;
            uint32_t flags = (uint32_t)arg1;
            file_t *file = vfs_open(path, flags);
            if (file == NULL) {
                return SYSCALL_ERROR;
            }
            /* Return file descriptor (pointer for now) */
            return (uint64_t)file;
        }
        
        case SYS_CLOSE: {
            /* Close file */
            file_t *file = (file_t*)arg0;
            return vfs_close(file);
        }
        
        case SYS_GETPID: {
            /* Get process ID */
            process_t *proc = current_proc();
            if (proc != NULL) {
                return proc->pid;
            }
            return 0;
        }
        
        case SYS_YIELD: {
            /* Yield CPU */
            sched_yield();
            return 0;
        }
        
        default:
            printf("[SYSCALL] Unknown syscall: %u\n", (uint32_t)num);
            return SYSCALL_ERROR;
    }
}
