#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "../types.h"
#include "../process/process.h"

/* Number of priority levels for MLFQ */
#define MLFQ_LEVELS 3

/* Number of CPUs (for SMP support) */
#define MAX_CPUS 4

/* Per-CPU scheduler data */
typedef struct cpu_info {
  int cpu_id;
  process_t *current_proc;  /* Currently running process */
  uint64_t idle_time;       /* Time spent in idle */
  uint64_t busy_time;       /* Time spent running processes */
} cpu_info_t;

/* Scheduler initialization */
void scheduler_init(void);

/* Start the scheduler */
void scheduler(void) __attribute__((noreturn));

/* Add a process to the ready queue */
void sched_add(process_t *proc);

/* Remove current process and schedule next */
void sched_yield(void);

/* Timer tick for preemption */
void sched_tick(void);

/* Get current running process */
process_t* current_proc(void);

/* Set process priority */
void sched_set_priority(process_t *proc, int priority);

/* Set process scheduling policy */
void sched_set_policy(process_t *proc, sched_policy_t policy);

/* Get CPU info for current CPU */
cpu_info_t* current_cpu_info(void);

/* Get process statistics */
void sched_get_stats(process_t *proc, proc_stats_t *stats);

#endif /* _SCHEDULER_H */
