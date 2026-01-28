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

process_t* process_alloc(void) {
    /* Find free slot */
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (proc_table[i].state == PROC_UNUSED) {
            proc_table[i].pid = next_pid++;
            proc_table[i].state = PROC_RUNNABLE;
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
