#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "../types.h"
#include "../process/process.h"

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

#endif /* _SCHEDULER_H */
