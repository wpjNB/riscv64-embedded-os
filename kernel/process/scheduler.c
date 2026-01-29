#include "scheduler.h"
#include "../printf.h"
#include "../riscv.h"

/* External assembly function for context switching */
extern void swtch(context_t *old, context_t *new);

#define MAX_PROCESSES 64

/* Time slices for different queue levels (in ticks) */
static const uint64_t queue_time_slices[NUM_QUEUE_LEVELS] = {
    5,   /* Level 0: Highest priority, shortest time slice */
    10,  /* Level 1 */
    20,  /* Level 2 */
    40   /* Level 3: Lowest priority, longest time slice */
};

/* Multi-level feedback queue */
typedef struct mlfq {
    process_t *queue[MAX_PROCESSES];
    int head;
    int tail;
    int size;
} mlfq_t;

/* Real-time queue for RT processes */
typedef struct rt_queue {
    process_t *queue[MAX_PROCESSES];
    int size;
} rt_queue_t;

/* Per-CPU scheduler data */
typedef struct cpu_sched {
    process_t *current;        /* Current running process */
    process_t *idle;           /* Idle process for this CPU */
    int cpu_id;                /* CPU ID */
    uint64_t ticks;            /* Local tick counter */
    uint64_t idle_ticks;       /* Time spent in idle */
} cpu_sched_t;

/* Global scheduler data */
static mlfq_t ready_queues[NUM_QUEUE_LEVELS];
static rt_queue_t rt_queue;
static cpu_sched_t cpu_data[MAX_CPUS];
static int num_cpus = 1;  /* Start with 1 CPU */
static process_t idle_processes[MAX_CPUS];

/* Current CPU (simplified for single core initially) */
static int current_cpu = 0;

/* Initialize an MLFQ queue */
static void mlfq_init(mlfq_t *q) {
    q->head = 0;
    q->tail = 0;
    q->size = 0;
}

/* Add process to MLFQ queue */
static void mlfq_enqueue(mlfq_t *q, process_t *proc) {
    if (proc == NULL || q->size >= MAX_PROCESSES) {
        return;
    }
    q->queue[q->tail] = proc;
    q->tail = (q->tail + 1) % MAX_PROCESSES;
    q->size++;
}

/* Remove process from MLFQ queue */
static process_t* mlfq_dequeue(mlfq_t *q) {
    if (q->size == 0) {
        return NULL;
    }
    process_t *proc = q->queue[q->head];
    q->head = (q->head + 1) % MAX_PROCESSES;
    q->size--;
    return proc;
}

/* Add process to RT queue (priority sorted) */
static void rt_enqueue(rt_queue_t *q, process_t *proc) {
    if (proc == NULL || q->size >= MAX_PROCESSES) {
        return;
    }
    
    /* Find insertion point based on priority (lower value = higher priority) */
    int i;
    for (i = q->size; i > 0; i--) {
        if (q->queue[i-1]->priority <= proc->priority) {
            break;
        }
        q->queue[i] = q->queue[i-1];
    }
    q->queue[i] = proc;
    q->size++;
}

/* Remove highest priority process from RT queue */
static process_t* rt_dequeue(rt_queue_t *q) {
    if (q->size == 0) {
        return NULL;
    }
    process_t *proc = q->queue[0];
    q->size--;
    for (int i = 0; i < q->size; i++) {
        q->queue[i] = q->queue[i+1];
    }
    return proc;
}

/* Initialize idle process */
static void init_idle_process(int cpu_id) {
    process_t *idle = &idle_processes[cpu_id];
    idle->pid = 0;  /* PID 0 for idle */
    idle->state = PROC_RUNNABLE;
    idle->priority = PRIORITY_MAX;  /* Lowest priority */
    idle->policy = SCHED_IDLE;
    idle->queue_level = NUM_QUEUE_LEVELS - 1;
    idle->cpu_id = cpu_id;
    idle->cpu_affinity = (1ULL << cpu_id);  /* Tied to specific CPU */
    
    /* Copy "idle" string to name */
    const char *name = "idle";
    int i;
    for (i = 0; name[i] != '\0' && i < 31; i++) {
        idle->name[i] = name[i];
    }
    idle->name[i] = '\0';
    
    /* Initialize stats */
    idle->stats.cpu_time = 0;
    idle->stats.context_switches = 0;
    idle->stats.start_time = get_ticks();
    idle->stats.last_run = 0;
    
    cpu_data[cpu_id].idle = idle;
}

/* Initialize the scheduler */
void scheduler_init(void) {
    printf("[SCHED] Initializing advanced scheduler\n");
    printf("[SCHED] Multi-level feedback queue (MLFQ) with %d levels\n", NUM_QUEUE_LEVELS);
    printf("[SCHED] Real-time scheduling support enabled\n");
    printf("[SCHED] SMP support: %d CPU(s)\n", num_cpus);
    
    /* Initialize MLFQ queues */
    for (int i = 0; i < NUM_QUEUE_LEVELS; i++) {
        mlfq_init(&ready_queues[i]);
        printf("[SCHED] Queue %d: time slice = %lu ticks\n", i, queue_time_slices[i]);
    }
    
    /* Initialize RT queue */
    rt_queue.size = 0;
    
    /* Initialize per-CPU data */
    for (int i = 0; i < num_cpus; i++) {
        cpu_data[i].current = NULL;
        cpu_data[i].cpu_id = i;
        cpu_data[i].ticks = 0;
        cpu_data[i].idle_ticks = 0;
        init_idle_process(i);
    }
    
    printf("[SCHED] Scheduler initialized\n");
}

/* Add a process to the ready queue */
void sched_add(process_t *proc) {
    if (proc == NULL) {
        return;
    }
    
    proc->state = PROC_RUNNABLE;
    
    /* Route based on scheduling policy */
    if (proc->policy == SCHED_FIFO || proc->policy == SCHED_RR) {
        /* Real-time process */
        rt_enqueue(&rt_queue, proc);
    } else if (proc->policy == SCHED_IDLE) {
        /* Idle process - don't queue it */
        return;
    } else {
        /* Normal process - add to MLFQ */
        int level = proc->queue_level;
        if (level < 0) level = 0;
        if (level >= NUM_QUEUE_LEVELS) level = NUM_QUEUE_LEVELS - 1;
        mlfq_enqueue(&ready_queues[level], proc);
    }
}

/* Get next process to run (priority scheduling) */
static process_t* sched_next(int cpu_id) {
    process_t *proc = NULL;
    
    /* First check RT queue (highest priority) */
    if (rt_queue.size > 0) {
        proc = rt_dequeue(&rt_queue);
        if (proc != NULL) {
            return proc;
        }
    }
    
    /* Then check MLFQ levels from highest to lowest */
    for (int level = 0; level < NUM_QUEUE_LEVELS; level++) {
        if (ready_queues[level].size > 0) {
            proc = mlfq_dequeue(&ready_queues[level]);
            if (proc != NULL) {
                /* Reset time slice for this level */
                proc->time_slice = queue_time_slices[level];
                return proc;
            }
        }
    }
    
    /* No ready processes, return idle process */
    return cpu_data[cpu_id].idle;
}

/* Get current running process */
process_t* current_proc(void) {
    return cpu_data[current_cpu].current;
}

/* Get current CPU ID */
int sched_cpu_id(void) {
    return current_cpu;
}

/* Set process priority */
void sched_set_priority(process_t *proc, int priority) {
    if (proc == NULL) return;
    
    if (priority < PRIORITY_MIN) priority = PRIORITY_MIN;
    if (priority > PRIORITY_MAX) priority = PRIORITY_MAX;
    
    proc->priority = priority;
    proc->dynamic_priority = priority;
    
    /* Determine queue level based on priority */
    if (priority <= PRIORITY_RT_MAX) {
        /* Real-time priority */
        proc->policy = SCHED_RR;
    } else {
        /* Normal priority - map to queue level */
        int range = (PRIORITY_MAX - PRIORITY_NORMAL_MIN + 1);
        int level = ((priority - PRIORITY_NORMAL_MIN) * NUM_QUEUE_LEVELS) / range;
        if (level >= NUM_QUEUE_LEVELS) level = NUM_QUEUE_LEVELS - 1;
        proc->queue_level = level;
    }
}

/* Set process policy */
void sched_set_policy(process_t *proc, sched_policy_t policy) {
    if (proc == NULL) return;
    proc->policy = policy;
}

/* Context switch from old to new process */
static void context_switch(process_t *old, process_t *new) {
    if (old == new) {
        return;
    }
    
    int cpu_id = current_cpu;
    
    /* Update statistics for old process */
    if (old != NULL && old->state == PROC_RUNNING) {
        old->stats.context_switches++;
        
        /* Handle MLFQ queue demotion for normal processes */
        if (old->policy == SCHED_NORMAL && old != cpu_data[cpu_id].idle) {
            /* If time slice expired, demote to lower priority queue */
            if (old->time_slice == 0 && old->queue_level < NUM_QUEUE_LEVELS - 1) {
                old->queue_level++;
            }
            old->state = PROC_RUNNABLE;
            sched_add(old);  /* Add back to ready queue */
        } else if (old->policy == SCHED_RR && old != cpu_data[cpu_id].idle) {
            /* RT round-robin - re-add to RT queue */
            old->state = PROC_RUNNABLE;
            sched_add(old);
        }
    }
    
    /* Load new process state */
    if (new != NULL) {
        new->state = PROC_RUNNING;
        new->stats.last_run = get_ticks();
        new->stats.context_switches++;
        new->cpu_id = cpu_id;
        cpu_data[cpu_id].current = new;
        
        /* Switch page table if different and not null */
        if (new->pagetable != NULL) {
            uint64_t satp = (8UL << 60) | ((uint64_t)new->pagetable >> 12);
            w_satp(satp);
            sfence_vma();
        }
        
        /* Perform actual context switch via assembly */
        if (old != NULL) {
            swtch(&old->context, &new->context);
        }
    }
}

/* Yield CPU to next process */
void sched_yield(void) {
    int cpu_id = current_cpu;
    process_t *old = cpu_data[cpu_id].current;
    process_t *new = sched_next(cpu_id);
    
    if (new != NULL) {
        context_switch(old, new);
    }
}

/* Timer tick handler for preemption */
void sched_tick(void) {
    int cpu_id = current_cpu;
    process_t *proc = cpu_data[cpu_id].current;
    
    /* Increment global tick counter */
    tick_increment();
    cpu_data[cpu_id].ticks++;
    
    if (proc == NULL) {
        return;
    }
    
    /* Update CPU time statistics */
    proc->stats.cpu_time++;
    
    /* Track idle time */
    if (proc == cpu_data[cpu_id].idle) {
        cpu_data[cpu_id].idle_ticks++;
    }
    
    /* Decrement time slice */
    if (proc->time_slice > 0) {
        proc->time_slice--;
    }
    
    /* Check if time slice expired */
    if (proc->time_slice == 0) {
        /* Preempt current process */
        if (proc->policy == SCHED_NORMAL) {
            /* For normal processes, this will trigger queue demotion */
            sched_yield();
        } else if (proc->policy == SCHED_RR) {
            /* RT round-robin preemption */
            sched_yield();
        }
        /* SCHED_FIFO processes don't get preempted by timer */
    }
    
    /* Aging mechanism: periodically boost all processes back to highest queue */
    /* This prevents starvation */
    if (get_ticks() % 100 == 0) {
        /* Boost all processes in lower queues */
        for (int level = 1; level < NUM_QUEUE_LEVELS; level++) {
            while (ready_queues[level].size > 0) {
                process_t *p = mlfq_dequeue(&ready_queues[level]);
                if (p != NULL && p->policy == SCHED_NORMAL) {
                    p->queue_level = 0;  /* Boost to highest queue */
                    mlfq_enqueue(&ready_queues[0], p);
                }
            }
        }
    }
}

/* Print scheduler statistics */
void sched_print_stats(void) {
    printf("\n[SCHED] Scheduler Statistics:\n");
    printf("========================================\n");
    
    for (int cpu_id = 0; cpu_id < num_cpus; cpu_id++) {
        printf("CPU %d:\n", cpu_id);
        printf("  Total ticks: %lu\n", cpu_data[cpu_id].ticks);
        printf("  Idle ticks: %lu\n", cpu_data[cpu_id].idle_ticks);
        
        if (cpu_data[cpu_id].ticks > 0) {
            uint64_t busy_ticks = cpu_data[cpu_id].ticks - cpu_data[cpu_id].idle_ticks;
            uint64_t usage = (busy_ticks * 100) / cpu_data[cpu_id].ticks;
            printf("  CPU Usage: %lu%%\n", usage);
        }
        
        if (cpu_data[cpu_id].current != NULL) {
            printf("  Current process: %s (PID %lu)\n", 
                   cpu_data[cpu_id].current->name,
                   cpu_data[cpu_id].current->pid);
        }
    }
    
    printf("\nQueue Status:\n");
    printf("  RT Queue: %d processes\n", rt_queue.size);
    for (int i = 0; i < NUM_QUEUE_LEVELS; i++) {
        printf("  Queue %d: %d processes (time slice: %lu)\n", 
               i, ready_queues[i].size, queue_time_slices[i]);
    }
    printf("========================================\n");
}

/* Main scheduler loop (never returns) */
void scheduler(void) {
    printf("[SCHED] Starting scheduler\n");
    
    int cpu_id = current_cpu;
    
    while (1) {
        /* Get next process */
        process_t *proc = sched_next(cpu_id);
        
        if (proc != NULL) {
            /* Switch to process */
            context_switch(NULL, proc);
            
            /* Run the process (would jump to process code here) */
            /* For now, just yield back */
            wfi();  /* Wait for interrupt */
        } else {
            /* Should not happen since we have idle process */
            wfi();
        }
    }
}
