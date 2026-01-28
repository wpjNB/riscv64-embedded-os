# 新功能实现文档 / New Features Implementation

本文档描述了 RISC-V 64-bit 嵌入式操作系统的新增强功能。
This document describes the newly implemented enhancement features in the RISC-V 64-bit Embedded OS.

## 虚拟内存 (SV39) / Virtual Memory (SV39)

### 功能 / Features
- **三级页表**: 实现了 SV39 分页机制，支持 512GB 虚拟地址空间
- **用户/内核分离**: 完全隔离用户态和内核态内存
- **页表管理**: 支持动态页表映射、取消映射和地址转换
- **TLB 管理**: 使用 sfence.vma 指令刷新 TLB

**Three-level page table**: Implements SV39 paging with 512GB virtual address space
**User/kernel separation**: Complete isolation between user and kernel memory
**Page table management**: Dynamic page mapping, unmapping, and address translation
**TLB management**: TLB flush using sfence.vma instruction

### 文件 / Files
- `kernel/mm/vm.h` - 虚拟内存接口 / Virtual memory interface
- `kernel/mm/vm.c` - 虚拟内存实现 / Virtual memory implementation

### API
```c
void vm_init(void);                        // 初始化虚拟内存系统
void kvminit(void);                        // 创建内核页表
void kvminithart(void);                    // 启用 SV39 分页
pagetable_t vm_create_user_pagetable(void); // 创建用户页表
int mappages(pagetable_t pt, uint64_t va, uint64_t size, uint64_t pa, int perm);
void unmappages(pagetable_t pt, uint64_t va, uint64_t size);
uint64_t walkaddr(pagetable_t pt, uint64_t va);
```

## 调度器 / Scheduler

### 功能 / Features
- **轮转调度**: 实现了公平的轮转调度算法
- **时间片抢占**: 基于定时器中断的抢占式调度
- **上下文切换**: 完整的进程上下文切换支持
- **就绪队列**: 循环队列管理就绪进程

**Round-robin scheduling**: Fair round-robin scheduling algorithm
**Time-slice preemption**: Preemptive scheduling based on timer interrupts
**Context switching**: Full process context switching support
**Ready queue**: Circular queue for managing ready processes

### 文件 / Files
- `kernel/process/scheduler.h` - 调度器接口 / Scheduler interface
- `kernel/process/scheduler.c` - 调度器实现 / Scheduler implementation

### API
```c
void scheduler_init(void);      // 初始化调度器
void scheduler(void);           // 主调度循环
void sched_add(process_t *proc); // 添加进程到就绪队列
void sched_yield(void);         // 主动让出 CPU
void sched_tick(void);          // 定时器中断处理
process_t* current_proc(void);  // 获取当前进程
```

### 配置 / Configuration
- **时间片**: 10 个定时器滴答 / Time slice: 10 timer ticks
- **最大进程数**: 64 / Maximum processes: 64

## 用户空间支持 / User Space Support

### ELF 加载器 / ELF Loader

#### 功能 / Features
- **ELF64 格式**: 支持 RISC-V 64-bit ELF 可执行文件
- **段加载**: 加载程序段到虚拟内存
- **权限管理**: 根据段标志设置页面权限 (R/W/X)
- **BSS 初始化**: 正确处理 BSS 段的零初始化

**ELF64 format**: Support for RISC-V 64-bit ELF executables
**Segment loading**: Load program segments to virtual memory
**Permission management**: Set page permissions based on segment flags (R/W/X)
**BSS initialization**: Proper zero-initialization of BSS segment

#### 文件 / Files
- `kernel/process/elf.h` - ELF 格式定义 / ELF format definitions
- `kernel/process/elf.c` - ELF 加载器实现 / ELF loader implementation

#### API
```c
int elf_load(const uint8_t *binary, size_t size, uint64_t *entry);
int elf_validate(const Elf64_Ehdr *ehdr);
```

### 系统调用 / System Calls

#### 新增系统调用 / New System Calls
- `SYS_OPEN (5)` - 打开文件 / Open file
- `SYS_CLOSE (6)` - 关闭文件 / Close file
- `SYS_GETPID (7)` - 获取进程 ID / Get process ID
- `SYS_YIELD (8)` - 主动让出 CPU / Yield CPU

#### 文件 / Files
- `kernel/syscall/syscall.h` - 系统调用定义 / System call definitions
- `kernel/syscall/syscall.c` - 系统调用实现 / System call implementation

## 文件系统 / File System

### VFS 层 / VFS Layer

#### 功能 / Features
- **统一接口**: 为不同文件系统提供统一的访问接口
- **设备文件**: 支持字符设备和块设备文件
- **inode 管理**: 管理文件 inode 和引用计数
- **文件描述符**: 维护打开的文件描述符

**Unified interface**: Provides uniform access interface for different file systems
**Device files**: Support for character and block device files
**inode management**: Manage file inodes and reference counting
**File descriptors**: Maintain open file descriptors

#### 文件 / Files
- `kernel/fs/vfs.h` - VFS 接口 / VFS interface
- `kernel/fs/vfs.c` - VFS 实现 / VFS implementation

#### API
```c
void vfs_init(void);
inode_t* vfs_create_inode(uint32_t type);
void vfs_destroy_inode(inode_t *inode);
file_t* vfs_open(const char *path, uint32_t flags);
int vfs_close(file_t *file);
int vfs_read(file_t *file, void *buf, size_t count);
int vfs_write(file_t *file, const void *buf, size_t count);
int vfs_register_device(const char *name, file_ops_t *ops);
```

### 简单文件系统 / Simple File System

#### 功能 / Features
- **内存文件系统**: 基于内存的简单文件系统
- **inode 管理**: 最多支持 64 个文件
- **块管理**: 4KB 块大小，直接块寻址
- **基本操作**: 支持创建、删除、读写文件

**In-memory file system**: Simple memory-based file system
**inode management**: Support up to 64 files
**Block management**: 4KB block size with direct block addressing
**Basic operations**: Support create, delete, read, write operations

#### 文件 / Files
- `kernel/fs/simplefs.h` - 简单文件系统接口 / Simple FS interface
- `kernel/fs/simplefs.c` - 简单文件系统实现 / Simple FS implementation

#### API
```c
void sfs_init(void);
int sfs_format(uint32_t num_blocks);
int sfs_create(const char *name, uint32_t type);
int sfs_delete(const char *name);
int sfs_read(uint32_t ino, void *buf, uint32_t offset, uint32_t size);
int sfs_write(uint32_t ino, const void *buf, uint32_t offset, uint32_t size);
```

#### 配置 / Configuration
- **块大小**: 4096 字节 / Block size: 4096 bytes
- **最大文件数**: 64 / Maximum files: 64
- **最大文件名长度**: 28 字符 / Maximum filename length: 28 characters

## 进程结构增强 / Process Structure Enhancement

### 上下文结构 / Context Structure
```c
typedef struct context {
    uint64_t ra;   // 返回地址 / Return address
    uint64_t sp;   // 栈指针 / Stack pointer
    uint64_t s0-s11; // 保存的寄存器 / Saved registers
} context_t;
```

### 进程结构 / Process Structure
```c
typedef struct process {
    uint64_t pid;          // 进程 ID / Process ID
    proc_state_t state;    // 进程状态 / Process state
    uint64_t *pagetable;   // 页表 / Page table
    context_t context;     // 上下文 / Context
    uint64_t kernel_sp;    // 内核栈指针 / Kernel stack pointer
    uint64_t user_sp;      // 用户栈指针 / User stack pointer
    char name[32];         // 进程名 / Process name
} process_t;
```

## 系统初始化流程 / System Initialization Flow

1. **内存管理初始化** / Memory management initialization (`mm_init()`)
2. **虚拟内存初始化** / Virtual memory initialization (`vm_init()`)
3. **启用 SV39 分页** / Enable SV39 paging (`kvminithart()`)
4. **陷阱处理初始化** / Trap handling initialization (`trap_init()`)
5. **调度器初始化** / Scheduler initialization (`scheduler_init()`)
6. **VFS 初始化** / VFS initialization (`vfs_init()`)
7. **简单文件系统初始化** / Simple FS initialization (`sfs_init()`, `sfs_format()`)

## 使用示例 / Usage Examples

### 虚拟内存 / Virtual Memory
```c
// 创建用户页表
pagetable_t pt = vm_create_user_pagetable();

// 映射虚拟地址到物理地址
mappages(pt, 0x1000, 4096, pa, PTE_R | PTE_W | PTE_U);

// 查询物理地址
uint64_t pa = walkaddr(pt, 0x1000);
```

### 调度 / Scheduling
```c
// 创建新进程
process_t *proc = process_alloc();
strcpy(proc->name, "user_process");

// 添加到调度器
sched_add(proc);

// 主动让出 CPU
sched_yield();
```

### 文件系统 / File System
```c
// 创建文件
int ino = sfs_create("testfile", VFS_FILE);

// 打开文件（通过 VFS）
file_t *file = vfs_open("/testfile", 0);

// 读写操作
vfs_write(file, data, size);
vfs_read(file, buffer, size);

// 关闭文件
vfs_close(file);
```

## 技术细节 / Technical Details

### 虚拟内存布局 / Virtual Memory Layout
- **内核空间**: 0x80000000 - 0x88000000 (128MB)
- **用户空间**: 0x0 - MAXVA (最高 256GB)
- **设备映射**: UART (0x10000000), PLIC (0x0C000000), CLINT (0x02000000)

### 页表结构 / Page Table Structure
- **Level 2**: VPN[2] - 顶级页表 / Top-level page table
- **Level 1**: VPN[1] - 中级页表 / Mid-level page table
- **Level 0**: VPN[0] - 底级页表 / Leaf-level page table

每个页表包含 512 个 PTE (页表项)
Each page table contains 512 PTEs (Page Table Entries)

### 性能优化 / Performance Optimizations
- 使用 TLB 缓存地址转换 / Use TLB to cache address translations
- 按需分配页表 / Allocate page tables on demand
- 直接映射内核空间以提高性能 / Direct map kernel space for performance

## 未来工作 / Future Work

### 计划中的改进 / Planned Improvements
1. **完整的用户态执行** / Full user mode execution
   - 用户/内核态切换 / User/kernel mode switching
   - 系统调用陷阱处理 / System call trap handling

2. **进程间通信** / Inter-process communication
   - 管道 / Pipes
   - 信号 / Signals
   - 共享内存 / Shared memory

3. **高级文件系统功能** / Advanced file system features
   - 目录支持 / Directory support
   - 文件权限 / File permissions
   - 持久化存储 / Persistent storage

4. **内存管理优化** / Memory management optimization
   - 写时复制 (COW) / Copy-on-write (COW)
   - 内存换页 / Memory swapping
   - 内存碎片整理 / Memory defragmentation

5. **多核支持** / Multi-core support
   - SMP (对称多处理) / SMP (Symmetric Multi-Processing)
   - 每核心调度 / Per-core scheduling
   - 自旋锁 / Spinlocks

## 参考资料 / References
- RISC-V Privileged Architecture Specification
- RISC-V SV39 Paging Documentation
- xv6-riscv Operating System
- Linux Kernel Documentation
