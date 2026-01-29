#include "scheduler.h"

#include "../printf.h"
#include "../riscv.h"
#include "process.h"

/* External assembly function for context switching */
extern void switch_context(context_t *old, context_t *new);

#define MAX_PROCESSES 64
#define RT_TIME_SLICE 10  /* Time slice for real-time Round-Robin */

/* Multi-Level Feedback Queue (MLFQ) structure */
typedef struct mlfq {
  process_t *queue[MAX_PROCESSES];
  int head;
  int tail;
  int size;
  uint64_t time_slice;  /* Time slice for this level */
} mlfq_t;

/* Real-time process queue */
typedef struct rt_queue {
  process_t *queue[MAX_PROCESSES];
  int head;
  int tail;
  int size;
} rt_queue_t;

/* MLFQ levels (higher index = lower priority) */
static mlfq_t mlfq_levels[MLFQ_LEVELS];

/* Real-time ready queue (highest priority) */
static rt_queue_t rt_queue;

/* Idle process */
static process_t idle_process;
static uint8_t idle_stack[4096] __attribute__((aligned(16)));

/* Per-CPU information */
static cpu_info_t cpu_infos[MAX_CPUS];
static int num_cpus = 1;  /* Default to single CPU */

/* Current CPU ID (would be per-CPU variable in real SMP) */
static int current_cpu_id = 0;

/* Global tick counter */
static uint64_t global_tick = 0;

/* Helper functions for queue management */
static void mlfq_init(mlfq_t *q, uint64_t time_slice) {
  q->head = 0;
  q->tail = 0;
  q->size = 0;
  q->time_slice = time_slice;
}

static void mlfq_enqueue(mlfq_t *q, process_t *proc) {
  if (q->size >= MAX_PROCESSES) {
    return;
  }
  q->queue[q->tail] = proc;
  q->tail = (q->tail + 1) % MAX_PROCESSES;
  q->size++;
}

static process_t *mlfq_dequeue(mlfq_t *q) {
  if (q->size == 0) {
    return NULL;
  }
  process_t *proc = q->queue[q->head];
  q->head = (q->head + 1) % MAX_PROCESSES;
  q->size--;
  return proc;
}

static void rt_queue_init(rt_queue_t *q) {
  q->head = 0;
  q->tail = 0;
  q->size = 0;
}

static void rt_queue_enqueue(rt_queue_t *q, process_t *proc) {
  if (q->size >= MAX_PROCESSES) {
    return;
  }
  
  /* Insert based on priority (lower number = higher priority) */
  int insert_pos = q->tail;
  for (int i = 0; i < q->size; i++) {
    int idx = (q->head + i) % MAX_PROCESSES;
    if (q->queue[idx]->priority > proc->priority) {
      insert_pos = idx;
      break;
    }
  }
  
  /* Shift elements to make room */
  if (insert_pos != q->tail && q->size > 0) {
    for (int i = q->size; i > 0; i--) {
      int from = (q->head + i - 1) % MAX_PROCESSES;
      int to = (q->head + i) % MAX_PROCESSES;
      if (from >= insert_pos) {
        q->queue[to] = q->queue[from];
      }
    }
  }
  
  q->queue[insert_pos] = proc;
  q->tail = (q->tail + 1) % MAX_PROCESSES;
  q->size++;
}

static process_t *rt_queue_dequeue(rt_queue_t *q) {
  if (q->size == 0) {
    return NULL;
  }
  process_t *proc = q->queue[q->head];
  q->head = (q->head + 1) % MAX_PROCESSES;
  q->size--;
  return proc;
}

/* Idle process function */
static void idle_func(void) {
  while (1) {
    wfi();  /* Wait for interrupt */
  }
}

/* Initialize the idle process */
static void init_idle_process(void) {
  idle_process.pid = 0;  /* PID 0 for idle */
  idle_process.state = PROC_RUNNABLE;
  idle_process.pagetable = NULL;
  idle_process.priority = PRIO_MAX;
  idle_process.dynamic_priority = PRIO_MAX;
  idle_process.policy = SCHED_IDLE;
  idle_process.time_slice = 0;
  idle_process.total_time_slice = 0;
  idle_process.cpu_affinity = -1;
  idle_process.last_cpu = -1;
  idle_process.stats.cpu_time = 0;
  idle_process.stats.context_switches = 0;
  idle_process.stats.last_run_tick = 0;
  
  /* Set name */
  const char *name = "idle";
  int i;
  for (i = 0; i < 4; i++) {
    idle_process.name[i] = name[i];
  }
  idle_process.name[i] = '\0';
  
  /* Setup context to point to idle function */
  process_setup_context(&idle_process, idle_func, 
                        idle_stack + sizeof(idle_stack));
  
  printf("[SCHED] Idle process initialized (PID 0)\n");
}

/* Initialize the scheduler */
void scheduler_init(void) {
  printf("[SCHED] Initializing Multi-Level Feedback Queue scheduler\n");
  
  /* Initialize MLFQ levels with different time slices */
  mlfq_init(&mlfq_levels[0], 10);  /* High priority: 10 ticks */
  mlfq_init(&mlfq_levels[1], 20);  /* Medium priority: 20 ticks */
  mlfq_init(&mlfq_levels[2], 40);  /* Low priority: 40 ticks */
  
  printf("[SCHED] MLFQ Level 0: time slice = %u ticks\n", (uint32_t)mlfq_levels[0].time_slice);
  printf("[SCHED] MLFQ Level 1: time slice = %u ticks\n", (uint32_t)mlfq_levels[1].time_slice);
  printf("[SCHED] MLFQ Level 2: time slice = %u ticks\n", (uint32_t)mlfq_levels[2].time_slice);
  
  /* Initialize real-time queue */
  rt_queue_init(&rt_queue);
  printf("[SCHED] Real-time queue initialized\n");
  
  /* Initialize idle process */
  init_idle_process();
  
  /* Initialize per-CPU info */
  for (int i = 0; i < MAX_CPUS; i++) {
    cpu_infos[i].cpu_id = i;
    cpu_infos[i].current_proc = NULL;
    cpu_infos[i].idle_time = 0;
    cpu_infos[i].busy_time = 0;
  }
  
  global_tick = 0;
  
  printf("[SCHED] SMP support: %d CPUs\n", num_cpus);
  printf("[SCHED] Scheduler initialization complete\n");
}

/* Get current CPU info */
cpu_info_t* current_cpu_info(void) {
  return &cpu_infos[current_cpu_id];
}

/* Get current running process */
process_t *current_proc(void) { 
  return cpu_infos[current_cpu_id].current_proc;
}

/* Add a process to the appropriate ready queue */
void sched_add(process_t *proc) {
  if (proc == NULL) {
    return;
  }

  /* Check scheduling policy - don't add idle processes */
  if (proc->policy == SCHED_IDLE) {
    return;
  }

  proc->state = PROC_RUNNABLE;
  
  /* Check scheduling policy */
  if (proc->policy == SCHED_FIFO || proc->policy == SCHED_RR) {
    /* Real-time process */
    if (rt_queue.size >= MAX_PROCESSES) {
      printf("[SCHED] ERROR: RT queue full, cannot add process %s\n", proc->name);
      return;
    }
    rt_queue_enqueue(&rt_queue, proc);
  } else {
    /* Normal process - add to MLFQ based on dynamic priority */
    int level = 0;
    if (proc->dynamic_priority >= PRIO_NORMAL_MIN) {
      /* Map priority to MLFQ level */
      int prio_range = proc->dynamic_priority - PRIO_NORMAL_MIN;
      level = (prio_range * MLFQ_LEVELS) / (PRIO_NORMAL_MAX - PRIO_NORMAL_MIN + 1);
      if (level >= MLFQ_LEVELS) {
        level = MLFQ_LEVELS - 1;
      }
    }
    
    /* Check if queue has space */
    if (mlfq_levels[level].size >= MAX_PROCESSES) {
      printf("[SCHED] ERROR: MLFQ level %d full, cannot add process %s\n", level, proc->name);
      return;
    }
    
    /* Set time slice based on level */
    proc->time_slice = mlfq_levels[level].time_slice;
    proc->total_time_slice = mlfq_levels[level].time_slice;
    
    mlfq_enqueue(&mlfq_levels[level], proc);
  }
}

/* Get next process to run */
static process_t *sched_next(void) {
  process_t *proc = NULL;
  
  /* First check real-time queue */
  proc = rt_queue_dequeue(&rt_queue);
  if (proc != NULL) {
    return proc;
  }
  
  /* Then check MLFQ levels from highest to lowest priority */
  for (int i = 0; i < MLFQ_LEVELS; i++) {
    proc = mlfq_dequeue(&mlfq_levels[i]);
    if (proc != NULL) {
      return proc;
    }
  }
  
  /* No process ready, return idle */
  return &idle_process;
}

/* Context switch from old to new process */
static void context_switch(process_t *old, process_t *new) {
  if (old == new) {
    return;
  }

  cpu_info_t *cpu = current_cpu_info();
  
  /* Update statistics for old process */
  if (old != NULL && old->state == PROC_RUNNING) {
    /* Only increment context switches when switching away from running process */
    old->stats.context_switches++;
    
    /* Add old process back to ready queue if still runnable */
    if (old->policy != SCHED_IDLE) {
      old->state = PROC_RUNNABLE;
      sched_add(old);
    }
  }

  /* Load new process state */
  if (new != NULL) {
    new->state = PROC_RUNNING;
    new->last_cpu = current_cpu_id;
    new->stats.last_run_tick = global_tick;
    
    /* Note: context_switches for new process incremented when it's switched away from */
    
    cpu->current_proc = new;

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
  process_t *old = current_proc();
  process_t *new = sched_next();

  if (new != NULL) {
    context_switch(old, new);
  }
}

/* Timer tick handler for preemption */
void sched_tick(void) {
  global_tick++;
  
  process_t *proc = current_proc();
  cpu_info_t *cpu = current_cpu_info();
  
  if (proc != NULL && proc->state == PROC_RUNNING) {
    /* Update CPU time */
    proc->stats.cpu_time++;
    
    /* Update CPU busy time */
    if (proc->policy != SCHED_IDLE) {
      cpu->busy_time++;
    } else {
      cpu->idle_time++;
    }
    
    /* Handle time slice expiration */
    if (proc->policy == SCHED_NORMAL) {
      /* MLFQ: decrease time slice */
      if (proc->time_slice > 0) {
        proc->time_slice--;
      }
      
      /* If time slice expired, demote to lower priority level */
      if (proc->time_slice == 0) {
        /* Calculate current MLFQ level */
        int current_level = 0;
        if (proc->dynamic_priority >= PRIO_NORMAL_MIN) {
          int prio_range = proc->dynamic_priority - PRIO_NORMAL_MIN;
          current_level = (prio_range * MLFQ_LEVELS) / (PRIO_NORMAL_MAX - PRIO_NORMAL_MIN + 1);
          if (current_level >= MLFQ_LEVELS) {
            current_level = MLFQ_LEVELS - 1;
          }
        }
        
        /* Move to next lower level if possible */
        if (current_level < MLFQ_LEVELS - 1) {
          /* Calculate priority for next level */
          int next_level = current_level + 1;
          int prio_per_level = (PRIO_NORMAL_MAX - PRIO_NORMAL_MIN + 1) / MLFQ_LEVELS;
          proc->dynamic_priority = PRIO_NORMAL_MIN + (next_level * prio_per_level);
          
          /* Ensure we don't exceed max */
          if (proc->dynamic_priority > PRIO_NORMAL_MAX) {
            proc->dynamic_priority = PRIO_NORMAL_MAX;
          }
        }
        
        /* Preempt */
        sched_yield();
      }
    } else if (proc->policy == SCHED_RR) {
      /* Real-time Round-Robin: use fixed time slice */
      if (proc->time_slice > 0) {
        proc->time_slice--;
      }
      
      if (proc->time_slice == 0) {
        proc->time_slice = RT_TIME_SLICE; /* Reset time slice */
        sched_yield();
      }
    }
    /* SCHED_FIFO processes run until they yield or block */
  }
}

/* Set process priority */
void sched_set_priority(process_t *proc, int priority) {
  if (proc == NULL) {
    return;
  }
  
  if (priority < PRIO_MIN) {
    priority = PRIO_MIN;
  } else if (priority > PRIO_MAX) {
    priority = PRIO_MAX;
  }
  
  proc->priority = priority;
  proc->dynamic_priority = priority;
}

/* Set process scheduling policy */
void sched_set_policy(process_t *proc, sched_policy_t policy) {
  if (proc == NULL) {
    return;
  }
  
  proc->policy = policy;
  
  /* Set appropriate priority range and time slice */
  if (policy == SCHED_FIFO || policy == SCHED_RR) {
    /* Real-time: priority 0-99 */
    if (proc->priority >= PRIO_NORMAL_MIN || proc->priority > PRIO_RT_MAX) {
      /* Priority is outside RT range, set to default RT priority */
      proc->priority = PRIO_RT_MAX / 2;  /* Default RT priority (50) */
      proc->dynamic_priority = proc->priority;
    }
    
    proc->time_slice = RT_TIME_SLICE;
    proc->total_time_slice = RT_TIME_SLICE;
  } else if (policy == SCHED_IDLE) {
    proc->priority = PRIO_MAX;
    proc->dynamic_priority = PRIO_MAX;
  }
  /* For SCHED_NORMAL, keep existing priority */
}

/* Get process statistics */
void sched_get_stats(process_t *proc, proc_stats_t *stats) {
  if (proc == NULL || stats == NULL) {
    return;
  }
  
  stats->cpu_time = proc->stats.cpu_time;
  stats->context_switches = proc->stats.context_switches;
  stats->last_run_tick = proc->stats.last_run_tick;
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

      /* Process runs until it yields or is preempted */
      wfi(); /* Wait for interrupt */
    } else {
      /* Should never happen as we have idle process */
      wfi();
    }
  }
}
