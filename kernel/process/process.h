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

/* Process structure */
typedef struct process {
  uint64_t pid;
  proc_state_t state;  //就绪态，运行态，睡眠态（挂起）等
  uint64_t *pagetable;
  context_t context;  //上下文
  uint64_t kernel_sp;
  uint64_t user_sp;
  char name[32];
} process_t;

/* Process management functions */
void process_init(void);
process_t *process_alloc(void);
void process_free(process_t *p);

/* Setup initial context for a new process */
void process_setup_context(process_t *p, void (*entry)(void), void *stack_top);

#endif /* _PROCESS_H */
