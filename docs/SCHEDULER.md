# Advanced Scheduler Features

This document describes the advanced scheduling features implemented in the RISC-V 64-bit Embedded OS.

## Overview

The operating system now includes a comprehensive scheduling system with the following features:

1. **Priority-Based Scheduling** - Support for 140 priority levels (0-139)
2. **Multi-Level Feedback Queue (MLFQ)** - Adaptive scheduling with 3 priority levels
3. **Real-Time Scheduling** - Support for FIFO and Round-Robin real-time policies
4. **Process Statistics** - Track CPU time and context switch counts
5. **SMP Multi-Core Support** - Per-CPU structures and CPU affinity
6. **Idle Process** - Dedicated low-priority idle process
7. **Complete Context Switching** - Full register save/restore in assembly

## Priority Levels

The scheduler supports 140 priority levels:

- **0-99**: Real-time priorities (lower number = higher priority)
- **100-139**: Normal priorities
- **120**: Default priority for new processes

### Priority Ranges

```c
#define PRIO_MIN 0           // Highest priority
#define PRIO_MAX 139         // Lowest priority
#define PRIO_RT_MAX 99       // Max real-time priority
#define PRIO_NORMAL_MIN 100  // Min normal priority
#define PRIO_NORMAL_MAX 139  // Max normal priority
#define PRIO_DEFAULT 120     // Default priority
```

## Scheduling Policies

### 1. SCHED_NORMAL (Default)

Normal time-sharing scheduling using Multi-Level Feedback Queue (MLFQ).

**Behavior:**
- Processes are placed in one of 3 queues based on priority
- Each queue has a different time slice
- Processes that use their full time slice are demoted to lower priority
- Provides good interactive response while being fair to CPU-bound tasks

**MLFQ Levels:**
- Level 0 (High Priority): 10 ticks time slice
- Level 1 (Medium Priority): 20 ticks time slice
- Level 2 (Low Priority): 40 ticks time slice

### 2. SCHED_FIFO

Real-time First-In-First-Out scheduling.

**Behavior:**
- Process runs until it voluntarily yields or blocks
- No preemption by time slicing
- Only higher-priority RT processes can preempt
- Used for time-critical tasks

### 3. SCHED_RR

Real-time Round-Robin scheduling.

**Behavior:**
- Similar to SCHED_FIFO but with time slice preemption
- Fixed time slice of 10 ticks
- Preempted processes go to the back of the RT queue
- Better fairness than FIFO for RT tasks

### 4. SCHED_IDLE

Idle task scheduling.

**Behavior:**
- Lowest priority possible
- Only runs when no other process is ready
- Uses WFI (Wait For Interrupt) to save power
- System idle process uses this policy

## Process Statistics

Each process tracks the following statistics:

```c
typedef struct proc_stats {
  uint64_t cpu_time;        /* Total CPU time in ticks */
  uint64_t context_switches; /* Number of context switches */
  uint64_t last_run_tick;   /* Last time process ran */
} proc_stats_t;
```

### Getting Statistics

```c
process_t *proc = current_proc();
proc_stats_t stats;
sched_get_stats(proc, &stats);

printf("CPU time: %lu ticks\n", stats.cpu_time);
printf("Context switches: %lu\n", stats.context_switches);
```

## SMP Multi-Core Support

The scheduler includes infrastructure for multi-core support:

### Per-CPU Information

```c
typedef struct cpu_info {
  int cpu_id;               /* CPU identifier */
  process_t *current_proc;  /* Currently running process */
  uint64_t idle_time;       /* Time spent in idle */
  uint64_t busy_time;       /* Time spent running processes */
} cpu_info_t;
```

### CPU Affinity

Processes can be pinned to specific CPUs:

```c
process_t *proc = process_alloc();
proc->cpu_affinity = 0;  /* Pin to CPU 0 */
// -1 means any CPU (default)
```

### Current Implementation

- Supports up to 4 CPUs (configurable via MAX_CPUS)
- Currently runs in single-CPU mode (num_cpus = 1)
- Infrastructure ready for multi-core expansion

## Idle Process

The system includes a dedicated idle process (PID 0) that:

- Runs when no other process is ready
- Uses SCHED_IDLE policy
- Executes WFI (Wait For Interrupt) to save power
- Cannot be scheduled by user code

### Idle Process Implementation

```c
static void idle_func(void) {
  while (1) {
    wfi();  /* Wait for interrupt */
  }
}
```

## Context Switching

Complete context switching is implemented in assembly (`switch.S`):

### Saved Registers

The context structure saves all callee-saved registers:
- `ra` - Return address
- `sp` - Stack pointer
- `s0-s11` - Saved registers

### Context Switch Process

1. Save current process's registers to old context
2. Restore new process's registers from new context
3. Switch page table if different
4. Update process statistics
5. Return to new process via `ret` instruction

## API Functions

### Setting Priority

```c
void sched_set_priority(process_t *proc, int priority);
```

Sets the static priority of a process (0-139).

### Setting Policy

```c
void sched_set_policy(process_t *proc, sched_policy_t policy);
```

Sets the scheduling policy:
- `SCHED_NORMAL` - Normal time-sharing
- `SCHED_FIFO` - Real-time FIFO
- `SCHED_RR` - Real-time Round-Robin
- `SCHED_IDLE` - Idle scheduling

### Getting Statistics

```c
void sched_get_stats(process_t *proc, proc_stats_t *stats);
```

Retrieves process statistics.

### Getting Current CPU Info

```c
cpu_info_t* current_cpu_info(void);
```

Returns information about the current CPU.

## Usage Examples

### Creating a High-Priority Process

```c
process_t *proc = process_alloc();
sched_set_priority(proc, 100);  /* High priority */
sched_add(proc);
```

### Creating a Real-Time Process

```c
process_t *rt_proc = process_alloc();
sched_set_policy(rt_proc, SCHED_FIFO);
sched_set_priority(rt_proc, 50);  /* RT priority */
sched_add(rt_proc);
```

### Monitoring Process Performance

```c
process_t *proc = current_proc();
proc_stats_t stats;
sched_get_stats(proc, &stats);

printf("Process %s:\n", proc->name);
printf("  CPU time: %lu ticks\n", stats.cpu_time);
printf("  Context switches: %lu\n", stats.context_switches);
```

## Implementation Details

### Queue Management

- Real-time queue maintains priority order
- MLFQ levels use simple FIFO queues
- Queue operations are O(1) for enqueue/dequeue
- RT queue insertion is O(n) for priority ordering

### Timer Tick Handling

On each timer tick:
1. Increment global tick counter
2. Update current process CPU time
3. Decrement time slice for time-sliced policies
4. Check for preemption
5. Demote MLFQ processes if time slice expired
6. Update CPU statistics

### Preemption Rules

- RT processes preempt normal processes
- Higher-priority RT processes preempt lower-priority ones
- MLFQ processes preempt each other based on dynamic priority
- Idle process never preempts any other process

## Performance Considerations

### Time Complexity

- Process scheduling: O(1) for MLFQ, O(n) for RT queue
- Context switch: O(1)
- Statistics update: O(1)

### Space Complexity

- Per-process overhead: ~200 bytes (including statistics)
- Per-CPU overhead: ~40 bytes
- Queue overhead: 64 * 8 bytes per queue

## Future Enhancements

Potential improvements for future versions:

1. **Load Balancing** - Distribute processes across CPUs
2. **Dynamic Priority Adjustment** - Better MLFQ tuning
3. **Priority Inheritance** - For real-time systems
4. **CPU Hotplug** - Dynamic CPU management
5. **Advanced Statistics** - Per-queue wait times, etc.

## References

- RISC-V Calling Convention: Callee-saved registers (s0-s11, sp, ra)
- Multi-Level Feedback Queue: Classic scheduling algorithm
- Real-Time Scheduling: POSIX real-time extensions
- SMP Support: Per-CPU data structures

## Testing

See `kernel/main.c::test_scheduler_features()` for comprehensive tests of all scheduler features.

Run the tests:
```bash
make clean
make all
make run
```

The test will demonstrate:
- Process creation with different priorities
- Real-time scheduling policies
- Process statistics tracking
- MLFQ behavior
- CPU affinity
- Idle process operation
- Per-CPU information
