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

/* Priority levels */
#define PRIORITY_MIN 0
#define PRIORITY_MAX 139
#define PRIORITY_DEFAULT 100
#define PRIORITY_RT_MAX 99   /* Real-time priorities: 0-99 */
#define PRIORITY_NORMAL_MIN 100 /* Normal priorities: 100-139 */

/* Scheduling policy */
typedef enum {
    SCHED_NORMAL,  /* Normal round-robin */
    SCHED_FIFO,    /* Real-time FIFO */
    SCHED_RR,      /* Real-time round-robin */
    SCHED_IDLE     /* Idle priority */
} sched_policy_t;

/* Saved registers for context switch */
typedef struct context {
    uint64_t ra;  /* Return address */
    uint64_t sp;  /* Stack pointer */
    uint64_t s0;  /* Saved registers */
    uint64_t s1;
    uint64_t s2;
    uint64_t s3;
    uint64_t s4;
    uint64_t s5;
    uint64_t s6;
    uint64_t s7;
    uint64_t s8;
    uint64_t s9;
    uint64_t s10;
    uint64_t s11;
} context_t;

/* Process statistics */
typedef struct proc_stats {
    uint64_t cpu_time;         /* Total CPU time used (ticks) */
    uint64_t context_switches; /* Number of context switches */
    uint64_t start_time;       /* Process start time (ticks) */
    uint64_t last_run;         /* Last time process ran */
} proc_stats_t;

/* Process structure */
typedef struct process {
    uint64_t pid;
    proc_state_t state;
    uint64_t *pagetable;
    context_t context;
    uint64_t kernel_sp;
    uint64_t user_sp;
    char name[32];
    
    /* Scheduling fields */
    int priority;              /* Static priority (0-139) */
    int dynamic_priority;      /* Dynamic priority for MLFQ */
    sched_policy_t policy;     /* Scheduling policy */
    int queue_level;           /* Current queue level in MLFQ */
    uint64_t time_slice;       /* Remaining time slice */
    uint64_t cpu_affinity;     /* CPU affinity mask for SMP */
    int cpu_id;                /* Currently assigned CPU (-1 if none) */
    
    /* Statistics */
    proc_stats_t stats;
} process_t;

/* Process management functions */
void process_init(void);
process_t* process_alloc(void);
void process_free(process_t *p);

/* Statistics functions */
void process_get_stats(process_t *p, proc_stats_t *stats);
void process_print_stats(process_t *p);

/* Time functions */
uint64_t get_ticks(void);
void tick_increment(void);

#endif /* _PROCESS_H */
