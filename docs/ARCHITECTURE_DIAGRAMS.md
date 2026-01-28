# System Architecture Diagram / 系统架构图

## Overall Architecture / 整体架构

```
┌─────────────────────────────────────────────────────────────────┐
│                         User Space / 用户空间                    │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐          │
│  │ User Process │  │ User Process │  │ User Process │          │
│  │     (ELF)    │  │     (ELF)    │  │     (ELF)    │          │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘          │
│         │                 │                 │                   │
│         │                 │                 │                   │
│         └─────────────────┴─────────────────┘                   │
│                           │                                     │
│                    System Calls / 系统调用                      │
│                           │                                     │
├───────────────────────────┼─────────────────────────────────────┤
│                           ▼                                     │
│                    Kernel Space / 内核空间                      │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │              System Call Interface / 系统调用接口        │   │
│  │  read | write | open | close | fork | exec | getpid    │   │
│  └────────────────────┬────────────────────────────────────┘   │
│                       │                                         │
│  ┌────────────────────┴────────────────────────────────────┐   │
│  │            Process Management / 进程管理                 │   │
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │   │
│  │  │  Scheduler   │  │ ELF Loader   │  │   Process    │  │   │
│  │  │   调度器     │  │  ELF加载器   │  │     Table    │  │   │
│  │  │              │  │              │  │   进程表     │  │   │
│  │  │ - Round Robin│  │ - Validate   │  │ - PID        │  │   │
│  │  │ - Preemption │  │ - Load Segs  │  │ - State      │  │   │
│  │  │ - Context SW │  │ - Entry Pt   │  │ - Context    │  │   │
│  │  └──────────────┘  └──────────────┘  └──────────────┘  │   │
│  └────────────────────┬────────────────────────────────────┘   │
│                       │                                         │
│  ┌────────────────────┴────────────────────────────────────┐   │
│  │          File System / 文件系统                          │   │
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │   │
│  │  │     VFS      │  │  SimpleFS    │  │    Device    │  │   │
│  │  │  虚拟文件系统 │  │  简单文件系统│  │   Files      │  │   │
│  │  │              │  │              │  │   设备文件   │  │   │
│  │  │ - inode      │  │ - In-memory  │  │ - UART       │  │   │
│  │  │ - File Desc  │  │ - 64 files   │  │ - RTC        │  │   │
│  │  │ - Operations │  │ - 4KB blocks │  │ - Block Dev  │  │   │
│  │  └──────────────┘  └──────────────┘  └──────────────┘  │   │
│  └────────────────────┬────────────────────────────────────┘   │
│                       │                                         │
│  ┌────────────────────┴────────────────────────────────────┐   │
│  │        Memory Management / 内存管理                      │   │
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │   │
│  │  │   Physical   │  │   Virtual    │  │  Page Table  │  │   │
│  │  │    Memory    │  │    Memory    │  │   页表管理   │  │   │
│  │  │   物理内存   │  │   虚拟内存   │  │              │  │   │
│  │  │              │  │              │  │ - SV39       │  │   │
│  │  │ - Page Alloc │  │ - 512GB VA   │  │ - 3-level    │  │   │
│  │  │ - kmalloc    │  │ - User/Kern  │  │ - Walk       │  │   │
│  │  │ - Free list  │  │ - TLB        │  │ - Map/Unmap  │  │   │
│  │  └──────────────┘  └──────────────┘  └──────────────┘  │   │
│  └────────────────────┬────────────────────────────────────┘   │
│                       │                                         │
│  ┌────────────────────┴────────────────────────────────────┐   │
│  │          Trap Handling / 中断处理                        │   │
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │   │
│  │  │  Exceptions  │  │  Interrupts  │  │   Syscalls   │  │   │
│  │  │   异常处理   │  │   中断处理   │  │  系统调用陷阱│  │   │
│  │  │              │  │              │  │              │  │   │
│  │  │ - Page Fault │  │ - Timer      │  │ - ecall      │  │   │
│  │  │ - Illegal Op │  │ - External   │  │ - Context SW │  │   │
│  │  │ - Misaligned │  │ - Software   │  │ - Return     │  │   │
│  │  └──────────────┘  └──────────────┘  └──────────────┘  │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │              Hardware Drivers / 硬件驱动                 │   │
│  │    UART (串口)  |  RTC (时钟)  |  PLIC (中断控制器)      │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

## Virtual Memory Layout / 虚拟内存布局

```
User Space (Per Process) / 用户空间 (每个进程)
┌─────────────────────────┐ 0x0000000000000000
│                         │
│    User Code & Data     │  ← ELF segments loaded here
│     用户代码和数据      │
│                         │
├─────────────────────────┤ 0x0000000040000000 (1GB)
│                         │
│      User Heap          │  ← Dynamic allocation
│      用户堆             │
│                         │
├─────────────────────────┤
│          ...            │
│                         │
├─────────────────────────┤
│      User Stack         │  ← Grows downward
│      用户栈             │
│           ↓             │
└─────────────────────────┘ MAXVA (256GB)

Kernel Space (Global) / 内核空间 (全局)
┌─────────────────────────┐ 0x0000000080000000 (KERNBASE)
│    Kernel Code          │  ← .text section
│    内核代码             │
├─────────────────────────┤
│    Kernel Data          │  ← .data, .bss
│    内核数据             │
├─────────────────────────┤
│    Kernel Stack         │  ← 64KB
│    内核栈               │
├─────────────────────────┤
│    Kernel Heap          │  ← 1MB
│    内核堆               │
├─────────────────────────┤
│    Free Memory          │  ← Available for allocation
│    空闲内存             │
└─────────────────────────┘ 0x0000000088000000 (128MB total)

Device Memory / 设备内存
┌─────────────────────────┐ 0x0000000002000000
│      CLINT              │  ← Timer, software interrupts
│    核心本地中断器       │
└─────────────────────────┘
┌─────────────────────────┐ 0x000000000C000000
│      PLIC               │  ← Platform interrupt controller
│    平台级中断控制器     │
└─────────────────────────┘ 0x0000000010000000
┌─────────────────────────┐ 0x0000000010000000
│      UART               │  ← Serial console
│      串口控制器         │
└─────────────────────────┘
```

## Page Table Structure / 页表结构

```
Virtual Address (39 bits) / 虚拟地址 (39位)
┌──────┬──────┬──────┬────────────┐
│ VPN2 │ VPN1 │ VPN0 │   Offset   │
│ 9bit │ 9bit │ 9bit │   12 bit   │
└──┬───┴──┬───┴──┬───┴────────────┘
   │      │      │
   │      │      └──────────┐
   │      │                 │
   │      └─────────┐       │
   │                │       │
   ▼                ▼       ▼
┌─────────┐    ┌─────────┐ ┌─────────┐
│ Level 2 │───▶│ Level 1 │─│ Level 0 │──▶ Physical Page
│  Page   │    │  Page   │ │  Page   │   (+ Offset)
│  Table  │    │  Table  │ │  Table  │
│  (512)  │    │  (512)  │ │  (512)  │
└─────────┘    └─────────┘ └─────────┘

PTE Format (Page Table Entry) / 页表项格式
┌──────────────────────────────────────┬────────────┐
│         PPN (Physical Page Number)    │   Flags    │
│         物理页号 (44 bits)            │  标志 (10) │
│         [53:10]                      │   [9:0]    │
└──────────────────────────────────────┴────────────┘
                                         │││││││││└─ V (Valid)
                                         ││││││││└── R (Read)
                                         │││││││└─── W (Write)
                                         ││││││└──── X (Execute)
                                         │││││└───── U (User)
                                         ││││└────── G (Global)
                                         │││└─────── A (Accessed)
                                         ││└──────── D (Dirty)
                                         └└───────── Reserved
```

## Process State Machine / 进程状态机

```
                    process_alloc()
         ┌────────────────────────────────┐
         │                                │
         ▼                                │
    ┌─────────┐                      ┌─────────┐
    │ UNUSED  │                      │ ZOMBIE  │
    │  未使用 │                      │  僵尸   │
    └─────────┘                      └─────────┘
         │                                ▲
         │ sched_add()                    │
         │                                │ process_exit()
         ▼                                │
    ┌─────────┐   scheduler()        ┌─────────┐
    │RUNNABLE │──────────────────────▶│ RUNNING │
    │  就绪   │◀──────────────────────│  运行   │
    └─────────┘   sched_yield()       └─────────┘
         ▲            preemption            │
         │                                  │
         │                                  │ sleep()
         │          wakeup()                │
         │                                  ▼
         └────────────────────────────┌─────────┐
                                      │SLEEPING │
                                      │  睡眠   │
                                      └─────────┘
```

## Scheduler Timeline / 调度器时间线

```
Time        Process A    Process B    Process C    Scheduler Action
──────────────────────────────────────────────────────────────────
t=0         RUNNING      RUNNABLE     RUNNABLE     A scheduled
t=1         RUNNING      RUNNABLE     RUNNABLE     
t=2         RUNNING      RUNNABLE     RUNNABLE     
...
t=10        RUNNABLE     RUNNING      RUNNABLE     Timer interrupt, preempt A, schedule B
t=11        RUNNABLE     RUNNING      RUNNABLE     
t=12        RUNNABLE     RUNNING      RUNNABLE     
...
t=20        RUNNABLE     RUNNABLE     RUNNING      Timer interrupt, preempt B, schedule C
t=21        RUNNABLE     RUNNABLE     RUNNING      
...
t=30        RUNNING      RUNNABLE     RUNNABLE     Timer interrupt, preempt C, schedule A
            └───────┬─────────┬──────────────┘
                    │         │
                Time Slice   Time Slice
                 (10 ticks)  (10 ticks)
```

## File System Hierarchy / 文件系统层次

```
                    Application
                         │
                         │ open/close/read/write
                         ▼
                 ┌───────────────┐
                 │      VFS      │  ← Virtual File System
                 │  虚拟文件系统 │
                 └───────┬───────┘
                         │
         ┌───────────────┼───────────────┐
         │               │               │
         ▼               ▼               ▼
  ┌──────────┐    ┌──────────┐    ┌──────────┐
  │ SimpleFS │    │  Device  │    │ (Future) │
  │ 简单文件 │    │  Files   │    │  ext2/   │
  │  系统    │    │ 设备文件 │    │  FAT32   │
  └────┬─────┘    └────┬─────┘    └──────────┘
       │               │
       ▼               ▼
  ┌──────────┐    ┌──────────┐
  │  Memory  │    │  Drivers │
  │  Buffer  │    │   驱动   │
  └──────────┘    └──────────┘

File Structure / 文件结构:
┌─────────────────────────────────────┐
│        Superblock / 超级块          │
│  - Magic number                     │
│  - Block size / 块大小              │
│  - Total blocks / 总块数            │
├─────────────────────────────────────┤
│        Inode Table / inode表        │
│  ┌──────────────────────────────┐   │
│  │ Inode 1: file1.txt           │   │
│  │  - size, blocks, name        │   │
│  ├──────────────────────────────┤   │
│  │ Inode 2: file2.dat           │   │
│  │  - size, blocks, name        │   │
│  ├──────────────────────────────┤   │
│  │ ...                          │   │
│  └──────────────────────────────┘   │
├─────────────────────────────────────┤
│        Data Blocks / 数据块         │
│  ┌────────┬────────┬────────┐       │
│  │Block 0 │Block 1 │Block 2 │ ...   │
│  │ 4KB    │ 4KB    │ 4KB    │       │
│  └────────┴────────┴────────┘       │
└─────────────────────────────────────┘
```

## System Call Flow / 系统调用流程

```
User Process / 用户进程
    │
    │ 1. ecall instruction
    │    (trap to S-mode)
    ▼
┌─────────────────────────┐
│   Trap Entry            │
│   trap_entry (boot.S)   │
│   - Save context        │
│   - Call trap_handler   │
└────────┬────────────────┘
         │
         │ 2. Identify as syscall
         ▼
┌─────────────────────────┐
│   Trap Handler          │
│   trap_handler()        │
│   - Check scause        │
│   - Route to syscall    │
└────────┬────────────────┘
         │
         │ 3. Call syscall handler
         ▼
┌─────────────────────────┐
│   Syscall Handler       │
│   syscall_handler()     │
│   - Check syscall num   │
│   - Execute syscall     │
│   - Return result       │
└────────┬────────────────┘
         │
         │ 4. Return to user
         ▼
┌─────────────────────────┐
│   Trap Return           │
│   - Restore context     │
│   - sret instruction    │
└────────┬────────────────┘
         │
         ▼
User Process / 用户进程
    (continues execution)
```

## Memory Allocation Flow / 内存分配流程

```
kmalloc(size)                    alloc_page()
     │                                │
     ▼                                ▼
┌─────────────────┐            ┌─────────────────┐
│  Bump Allocator │            │  Page Allocator │
│  碰撞分配器     │            │  页面分配器     │
├─────────────────┤            ├─────────────────┤
│ heap_current    │            │ free_pages      │
│      │          │            │      │          │
│      ▼          │            │      ▼          │
│ ┌──────────┐   │            │ ┌────┐ ┌────┐  │
│ │Used│Free │   │            │ │Page│→│Page│  │
│ └────┴─────┘   │            │ └────┘ └────┘  │
│  heap_start    │            │ Linked list     │
│  heap_end      │            │ of free pages   │
└─────────────────┘            └─────────────────┘
     │                                │
     ▼                                ▼
Return pointer                   Return 4KB page
返回指针                         返回 4KB 页面
```

## Interrupt Handling / 中断处理

```
                    Hardware Event / 硬件事件
                            │
                            ▼
                    ┌───────────────┐
                    │  Timer/Device │
                    │  定时器/设备  │
                    └───────┬───────┘
                            │
                            │ Interrupt signal
                            ▼
                    ┌───────────────┐
                    │     PLIC      │  ← Routes interrupt
                    │  中断控制器   │     to CPU
                    └───────┬───────┘
                            │
                            ▼
                    ┌───────────────┐
                    │  trap_entry   │  ← Save all registers
                    │  陷阱入口     │
                    └───────┬───────┘
                            │
                            ▼
                    ┌───────────────┐
                    │ trap_handler  │  ← Identify interrupt
                    │  陷阱处理器   │     type
                    └───────┬───────┘
                            │
                ┌───────────┴───────────┐
                │                       │
                ▼                       ▼
        ┌──────────────┐        ┌──────────────┐
        │    Timer     │        │   External   │
        │   Interrupt  │        │   Interrupt  │
        ├──────────────┤        ├──────────────┤
        │ sched_tick() │        │ Handle device│
        │ Preemption   │        │ interrupt    │
        └──────┬───────┘        └──────┬───────┘
               │                       │
               └───────────┬───────────┘
                           │
                           ▼
                   ┌───────────────┐
                   │  sret (return)│  ← Restore registers
                   │  返回用户态   │     and resume
                   └───────────────┘
```

## Notes / 注释

1. All diagrams show the logical structure, not physical layout
   所有图表显示逻辑结构，而非物理布局

2. Actual implementation may vary in details
   实际实现可能在细节上有所不同

3. Some features are simplified for clarity
   某些功能为清晰起见进行了简化

4. Full context switch requires assembly code
   完整的上下文切换需要汇编代码
