#include "scheduler.h"

#include "../printf.h"
#include "../riscv.h"

/* External assembly function for context switching */
extern void switch_context(context_t *old, context_t *new);

#define MAX_PROCESSES 64
#define TIME_SLICE 10 /* Timer ticks before preemption */

/* Ready queue for round-robin scheduling */
static process_t *ready_queue[MAX_PROCESSES];
static int queue_head = 0;
static int queue_tail = 0;
static int queue_size = 0;

/* Current running process */
static process_t *current = NULL;

/* Timer tick counter */
static uint64_t tick_count = 0;

/* Initialize the scheduler */
void scheduler_init(void) {
  printf("[SCHED] Initializing round-robin scheduler\n");
  printf("[SCHED] Time slice: %d ticks\n", TIME_SLICE);

  queue_head = 0;
  queue_tail = 0;
  queue_size = 0;
  current = NULL;
  tick_count = 0;
}

/* Add a process to the ready queue */
void sched_add(process_t *proc) {
  if (proc == NULL || queue_size >= MAX_PROCESSES) {
    return;
  }

  proc->state = PROC_RUNNABLE;
  ready_queue[queue_tail] = proc;
  queue_tail = (queue_tail + 1) % MAX_PROCESSES;
  queue_size++;
}

/* Get next process from ready queue */
static process_t *sched_next(void) {
  if (queue_size == 0) {
    return NULL;
  }

  process_t *proc = ready_queue[queue_head];
  queue_head = (queue_head + 1) % MAX_PROCESSES;
  queue_size--;

  return proc;
}

/* Get current running process */
process_t *current_proc(void) { return current; }

/* Context switch from old to new process */
static void context_switch(process_t *old, process_t *new) {
  if (old == new) {
    return;
  }

  /* Save old process state */
  if (old != NULL && old->state == PROC_RUNNING) {
    old->state = PROC_RUNNABLE;
    sched_add(old); /* Add back to ready queue */
  }

  /* Load new process state */
  if (new != NULL) {
    new->state = PROC_RUNNING;
    current = new;

    /* Switch page table if different */
    if (new->pagetable != NULL) {
      uint64_t satp = (8UL << 60) | ((uint64_t) new->pagetable >> 12);
      w_satp(satp);
      sfence_vma();
    }

    /* Perform full context switch (registers, SP, etc.) */
    context_t *old_ctx = (old != NULL) ? &old->context : NULL;
    context_t *new_ctx = &new->context;
    switch_context(old_ctx, new_ctx);
  }
}

/* Yield CPU to next process */
void sched_yield(void) {
  process_t *old = current;
  process_t *new = sched_next();

  if (new != NULL) {
    context_switch(old, new);
  }
}

/* Timer tick handler for preemption */
void sched_tick(void) {
  tick_count++;

  /* Check if time slice expired */
  if (tick_count % TIME_SLICE == 0 && current != NULL) {
    /* Preempt current process */
    sched_yield();
  }
}

/* Main scheduler loop (never returns) */
void scheduler(void) {
  printf("[SCHED] Starting scheduler\n");

  while (1) {
    /* Get next process */
    process_t *proc = sched_next();

    if (proc != NULL) {
      /* Switch to process */
      context_switch(NULL, proc);

      /* Run the process (would jump to process code here) */
      /* For now, just yield back */
      wfi(); /* Wait for interrupt */
    } else {
      /* No processes ready, idle */
      wfi();
    }
  }
}
