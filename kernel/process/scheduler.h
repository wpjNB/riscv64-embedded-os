#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "../types.h"
#include "../process/process.h"

/* Multi-level feedback queue configuration */
#define NUM_QUEUE_LEVELS 4
#define MAX_CPUS 8

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

/* Get current CPU ID */
int sched_cpu_id(void);

/* Set process priority */
void sched_set_priority(process_t *proc, int priority);

/* Set process policy */
void sched_set_policy(process_t *proc, sched_policy_t policy);

/* Print scheduler statistics */
void sched_print_stats(void);

#endif /* _SCHEDULER_H */
