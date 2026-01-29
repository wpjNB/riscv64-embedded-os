#include "process.h"

#include "../mm/mm.h"

#define MAX_PROCESSES 64

static process_t proc_table[MAX_PROCESSES];
static uint64_t next_pid = 1;

void process_init(void) {
  /* Initialize process table */
  for (int i = 0; i < MAX_PROCESSES; i++) {
    proc_table[i].state = PROC_UNUSED;
    proc_table[i].pid = 0;
  }
}

process_t *process_alloc(void) {
  /* Find free slot */
  for (int i = 0; i < MAX_PROCESSES; i++) {
    if (proc_table[i].state == PROC_UNUSED) {
      proc_table[i].pid = next_pid++;
      proc_table[i].state = PROC_RUNNABLE;
      proc_table[i].name[0] = '\0';
      
      /* Initialize scheduling fields */
      proc_table[i].priority = PRIO_DEFAULT;
      proc_table[i].dynamic_priority = PRIO_DEFAULT;
      proc_table[i].policy = SCHED_NORMAL;
      proc_table[i].time_slice = 0;
      proc_table[i].total_time_slice = 0;
      proc_table[i].cpu_affinity = -1; /* No affinity */
      proc_table[i].last_cpu = -1;
      
      /* Initialize statistics */
      proc_table[i].stats.cpu_time = 0;
      proc_table[i].stats.context_switches = 0;
      proc_table[i].stats.last_run_tick = 0;
      
      return &proc_table[i];
    }
  }
  return NULL;
}

void process_free(process_t *p) {
  if (p) {
    p->state = PROC_UNUSED;
    p->pid = 0;
  }
}

/* Setup initial context for a new process */
void process_setup_context(process_t *p, void (*entry)(void), void *stack_top) {
  if (p == NULL) {
    return;
  }

  /* Clear context */
  p->context.ra = (uint64_t)entry;     /* Entry point */
  p->context.sp = (uint64_t)stack_top; /* Stack pointer */
  p->context.s0 = 0;
  p->context.s1 = 0;
  p->context.s2 = 0;
  p->context.s3 = 0;
  p->context.s4 = 0;
  p->context.s5 = 0;
  p->context.s6 = 0;
  p->context.s7 = 0;
  p->context.s8 = 0;
  p->context.s9 = 0;
  p->context.s10 = 0;
  p->context.s11 = 0;
}
