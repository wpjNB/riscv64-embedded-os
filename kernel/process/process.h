#ifndef _PROCESS_H
#define _PROCESS_H

#include "../types.h"

/* Process states */
typedef enum {
    PROC_UNUSED,
    PROC_RUNNABLE,
    PROC_RUNNING,
    PROC_SLEEPING,
    PROC_ZOMBIE
} proc_state_t;

/* Process structure */
typedef struct process {
    uint64_t pid;
    proc_state_t state;
    uint64_t *pagetable;
    uint64_t context;
    uint64_t kernel_sp;
    uint64_t user_sp;
} process_t;

/* Process management functions */
void process_init(void);
process_t* process_alloc(void);
void process_free(process_t *p);

#endif /* _PROCESS_H */
