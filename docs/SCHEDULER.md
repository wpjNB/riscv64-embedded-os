# Advanced Scheduler Features

## Overview

This document describes the advanced scheduling features implemented in the RISC-V 64-bit Embedded OS.

## Features Implemented

### 1. Priority Scheduling

The scheduler now supports priority-based scheduling with the following priority ranges:

- **Priority Range**: 0-139
- **Real-Time Priorities**: 0-99 (lower number = higher priority)
- **Normal Priorities**: 100-139
- **Default Priority**: 100

**API:**
```c
void sched_set_priority(process_t *proc, int priority);
```

### 2. Multi-Level Feedback Queue (MLFQ)

The MLFQ implementation provides fairness and responsiveness:

- **Number of Queue Levels**: 4
- **Queue Time Slices**:
  - Level 0: 5 ticks (highest priority, shortest time slice)
  - Level 1: 10 ticks
  - Level 2: 20 ticks
  - Level 3: 40 ticks (lowest priority, longest time slice)

**Queue Management:**
- Processes start at the highest queue (level 0)
- When a process uses its entire time slice, it's demoted to the next lower queue
- **Aging Mechanism**: Every 100 ticks, all processes are boosted back to the highest queue to prevent starvation

### 3. Process Statistics

Each process now tracks detailed statistics:

```c
typedef struct proc_stats {
    uint64_t cpu_time;         /* Total CPU time used (ticks) */
    uint64_t context_switches; /* Number of context switches */
    uint64_t start_time;       /* Process start time (ticks) */
    uint64_t last_run;         /* Last time process ran */
} proc_stats_t;
```

**API:**
```c
void process_get_stats(process_t *p, proc_stats_t *stats);
void process_print_stats(process_t *p);
```

**Shell Commands:**
- `ps` - Display current process statistics
- `sched` - Display scheduler statistics including CPU usage

### 4. SMP Multi-Core Support

The scheduler includes infrastructure for SMP support:

- **Per-CPU Data Structure**: Each CPU has its own scheduler state
- **CPU Affinity**: Processes can be assigned to specific CPUs
- **Current CPU ID**: `sched_cpu_id()` returns the current CPU
- **Maximum CPUs**: 8 (configurable via MAX_CPUS)

```c
typedef struct cpu_sched {
    process_t *current;        /* Current running process */
    process_t *idle;           /* Idle process for this CPU */
    int cpu_id;                /* CPU ID */
    uint64_t ticks;            /* Local tick counter */
    uint64_t idle_ticks;       /* Time spent in idle */
} cpu_sched_t;
```

**Note**: Currently runs in single-CPU mode, but the infrastructure is in place for multi-core expansion.

### 5. Real-Time Scheduling

The scheduler supports real-time scheduling policies:

**Scheduling Policies:**
```c
typedef enum {
    SCHED_NORMAL,  /* Normal round-robin with MLFQ */
    SCHED_FIFO,    /* Real-time FIFO (no preemption) */
    SCHED_RR,      /* Real-time round-robin (preemptive) */
    SCHED_IDLE     /* Idle priority */
} sched_policy_t;
```

**RT Queue:**
- Separate priority queue for RT processes
- Sorted by priority (0 = highest)
- RT processes always run before normal processes
- SCHED_FIFO processes run until completion or blocking
- SCHED_RR processes can be preempted after their time slice

**API:**
```c
void sched_set_policy(process_t *proc, sched_policy_t policy);
```

### 6. Complete Context Switch Assembly

A full assembly implementation for context switching (`swtch.S`):

**Functions:**

1. **`swtch(context_t *old, context_t *new)`**
   - Saves all callee-saved registers (ra, sp, s0-s11)
   - Restores context from new process
   - Minimal overhead for fast context switches

2. **`switch_to_user(uint64_t satp, uint64_t pc, uint64_t sp)`**
   - Switches from kernel mode to user mode
   - Sets up user page table
   - Prepares for user execution

3. **`trap_vector_user()`**
   - Entry point for user mode traps
   - Saves all 31 registers to trap frame
   - Calls trap handler
   - Restores registers and returns to user mode

**Context Structure:**
```c
typedef struct context {
    uint64_t ra;   /* Return address */
    uint64_t sp;   /* Stack pointer */
    uint64_t s0;   /* Saved registers s0-s11 */
    uint64_t s1;
    /* ... s2-s11 ... */
} context_t;
```

### 7. Idle Process

Each CPU has its own idle process:

- **PID**: 0
- **Priority**: Maximum (139)
- **Policy**: SCHED_IDLE
- **CPU Affinity**: Tied to specific CPU
- **Behavior**: Runs only when no other processes are ready

**Idle Loop:**
- Executes `wfi` (wait for interrupt) instruction
- Tracks idle time for CPU utilization statistics
- Never blocks or sleeps

## Enhanced Process Structure

```c
typedef struct process {
    /* Basic fields */
    uint64_t pid;
    proc_state_t state;
    uint64_t *pagetable;
    context_t context;
    uint64_t kernel_sp;
    uint64_t user_sp;
    char name[32];
    
    /* Scheduling fields */
    int priority;              /* Static priority (0-139) */
    int dynamic_priority;      /* Dynamic priority for MLFQ */
    sched_policy_t policy;     /* Scheduling policy */
    int queue_level;           /* Current queue level in MLFQ */
    uint64_t time_slice;       /* Remaining time slice */
    uint64_t cpu_affinity;     /* CPU affinity mask for SMP */
    int cpu_id;                /* Currently assigned CPU (-1 if none) */
    
    /* Statistics */
    proc_stats_t stats;
} process_t;
```

## Scheduler Algorithm

The scheduler uses a hierarchical approach:

1. **Check RT Queue**: If any RT processes are ready, run the highest priority one
2. **Check MLFQ Levels**: Scan from level 0 to 3 for ready processes
3. **Run Idle**: If no processes are ready, run the idle process

**Time Slice Management:**
- Each process gets a time slice based on its queue level
- Time slice decreases on each tick
- When time slice reaches 0, process is preempted
- Normal processes are demoted to next queue level
- RT round-robin processes are re-queued at same priority

**Aging to Prevent Starvation:**
- Every 100 ticks, boost all normal processes to queue level 0
- Ensures long-running processes don't starve short ones
- Maintains fairness while rewarding interactive processes

## Usage Examples

### Setting Process Priority

```c
process_t *proc = process_alloc();
strcpy(proc->name, "high_priority");

// Set as real-time process
sched_set_priority(proc, 50);  // RT priority
sched_set_policy(proc, SCHED_FIFO);

sched_add(proc);
```

### Viewing Statistics

**In Shell:**
```
> sched
[SCHED] Scheduler Statistics:
========================================
CPU 0:
  Total ticks: 1234
  Idle ticks: 100
  CPU Usage: 91%
  Current process: test1 (PID 1)

Queue Status:
  RT Queue: 1 processes
  Queue 0: 2 processes (time slice: 5)
  Queue 1: 0 processes (time slice: 10)
  Queue 2: 0 processes (time slice: 20)
  Queue 3: 0 processes (time slice: 40)
========================================
```

### Process Statistics

```c
process_t *proc = current_proc();
process_print_stats(proc);

// Output:
// Process 1 (test1):
//   State: 2, Priority: 100, Policy: 0
//   CPU Time: 50 ticks
//   Context Switches: 10
//   Uptime: 100 ticks
//   CPU Usage: 50%
```

## Performance Considerations

- **Context Switch Overhead**: ~20-30 instructions (minimal)
- **Scheduling Decision**: O(1) for RT queue, O(n) worst case for MLFQ (where n = number of queues)
- **Statistics Tracking**: Minimal overhead, updated during context switch
- **Aging**: Periodic (every 100 ticks), adds O(m) where m = total processes

## Future Enhancements

1. **Full SMP Support**: Enable true multi-core scheduling with load balancing
2. **Priority Inheritance**: Prevent priority inversion problems
3. **CPU Affinity Control**: Allow processes to specify CPU affinity
4. **Dynamic Priority Adjustment**: Boost I/O-bound processes
5. **Deadline Scheduling**: Add earliest deadline first (EDF) policy
6. **Cgroup Support**: Resource limitation and management

## References

- RISC-V Privileged Architecture Specification
- xv6-riscv: MIT's teaching operating system
- Linux Completely Fair Scheduler (CFS)
- FreeBSD ULE Scheduler
- Solaris Multi-Level Feedback Queue

## Testing

The scheduler has been tested with:

1. Multiple processes with different priorities
2. Real-time vs. normal process scheduling
3. MLFQ queue demotion and aging
4. Statistics tracking accuracy
5. Context switching correctness

Run the test suite with:
```
> test
```

And monitor scheduler behavior with:
```
> sched
```
