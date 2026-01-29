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
#define PRIO_MIN 0
#define PRIO_MAX 139
#define PRIO_RT_MAX 99    /* Real-time priority range: 0-99 */
#define PRIO_NORMAL_MIN 100
#define PRIO_NORMAL_MAX 139
#define PRIO_DEFAULT 120

/* Scheduling policies */
typedef enum {
  SCHED_NORMAL,   /* Normal time-sharing scheduling */
  SCHED_FIFO,     /* Real-time FIFO */
  SCHED_RR,       /* Real-time Round-Robin */
  SCHED_IDLE      /* Idle task scheduling */
} sched_policy_t;

/* Saved registers for context switch */
typedef struct context {
  uint64_t ra; /* Return address */
  uint64_t sp; /* Stack pointer */
  uint64_t s0; /* Saved registers */
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
  uint64_t cpu_time;        /* Total CPU time in ticks */
  uint64_t context_switches; /* Number of context switches */
  uint64_t last_run_tick;   /* Last time process ran */
} proc_stats_t;

/* Process structure */
typedef struct process {
  uint64_t pid;
  proc_state_t state;        /* Process state: ready, running, sleeping, etc */
  uint64_t *pagetable;
  context_t context;         /* Context for context switching */
  uint64_t kernel_sp;
  uint64_t user_sp;
  char name[32];
  
  /* Scheduling related fields */
  int priority;              /* Static priority (0-139) */
  int dynamic_priority;      /* Dynamic priority for MLFQ */
  sched_policy_t policy;     /* Scheduling policy */
  uint64_t time_slice;       /* Remaining time slice */
  uint64_t total_time_slice; /* Total time slice for this priority level */
  int cpu_affinity;          /* CPU affinity (-1 for any CPU) */
  int last_cpu;              /* Last CPU this process ran on */
  
  /* Statistics */
  proc_stats_t stats;
} process_t;

/* Process management functions */
void process_init(void);
process_t *process_alloc(void);
void process_free(process_t *p);

/* Setup initial context for a new process */
void process_setup_context(process_t *p, void (*entry)(void), void *stack_top);

#endif /* _PROCESS_H */
