#include "process.h"
#include "../mm/mm.h"
#include "../printf.h"

#define MAX_PROCESSES 64

static process_t proc_table[MAX_PROCESSES];
static uint64_t next_pid = 1;
static uint64_t global_ticks = 0;  /* Global tick counter */

/* Get global tick counter */
uint64_t get_ticks(void) {
    return global_ticks;
}

/* Increment global tick counter */
void tick_increment(void) {
    global_ticks++;
}

void process_init(void) {
    /* Initialize process table */
    for (int i = 0; i < MAX_PROCESSES; i++) {
        proc_table[i].state = PROC_UNUSED;
        proc_table[i].pid = 0;
    }
}

process_t* process_alloc(void) {
    /* Find free slot */
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (proc_table[i].state == PROC_UNUSED) {
            proc_table[i].pid = next_pid++;
            proc_table[i].state = PROC_RUNNABLE;
            proc_table[i].name[0] = '\0';
            
            /* Initialize scheduling fields */
            proc_table[i].priority = PRIORITY_DEFAULT;
            proc_table[i].dynamic_priority = PRIORITY_DEFAULT;
            proc_table[i].policy = SCHED_NORMAL;
            proc_table[i].queue_level = 0;
            proc_table[i].time_slice = 0;
            proc_table[i].cpu_affinity = 0xFFFFFFFFFFFFFFFFULL; /* All CPUs */
            proc_table[i].cpu_id = -1;
            
            /* Initialize statistics */
            proc_table[i].stats.cpu_time = 0;
            proc_table[i].stats.context_switches = 0;
            proc_table[i].stats.start_time = global_ticks;
            proc_table[i].stats.last_run = 0;
            
            return &proc_table[i];
        }
    }
    return NULL;
}

void process_free(process_t *p) {
    if (p) {
        p->state = PROC_UNUSED;
        p->pid = 0;
        p->priority = 0;
        p->dynamic_priority = 0;
        p->queue_level = 0;
        p->time_slice = 0;
        p->cpu_id = -1;
        p->stats.cpu_time = 0;
        p->stats.context_switches = 0;
    }
}

/* Get process statistics */
void process_get_stats(process_t *p, proc_stats_t *stats) {
    if (p && stats) {
        stats->cpu_time = p->stats.cpu_time;
        stats->context_switches = p->stats.context_switches;
        stats->start_time = p->stats.start_time;
        stats->last_run = p->stats.last_run;
    }
}

/* Print process statistics */
void process_print_stats(process_t *p) {
    if (p && p->state != PROC_UNUSED) {
        uint64_t uptime = global_ticks - p->stats.start_time;
        printf("Process %lu (%s):\n", p->pid, p->name);
        printf("  State: %d, Priority: %d, Policy: %d\n", 
               p->state, p->priority, p->policy);
        printf("  CPU Time: %lu ticks\n", p->stats.cpu_time);
        printf("  Context Switches: %lu\n", p->stats.context_switches);
        printf("  Uptime: %lu ticks\n", uptime);
        if (uptime > 0) {
            uint64_t cpu_percent = (p->stats.cpu_time * 100) / uptime;
            printf("  CPU Usage: %lu%%\n", cpu_percent);
        }
    }
}
