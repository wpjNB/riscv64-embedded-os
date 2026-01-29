# 上下文切换实现文档

## 概述

已实现完整的进程上下文切换功能，支持多进程调度和寄存器状态保存/恢复。

## 核心文件

### 1. `kernel/process/switch.S` - 汇编上下文切换

```asm
switch_context(context_t *old, context_t *new)
```

**功能：**
- 保存当前进程的所有callee-saved寄存器到old
- 从new恢复寄存器状态
- 通过`ret`指令跳转到新进程

**保存的寄存器：**
- `ra`: 返回地址（程序计数器）
- `sp`: 栈指针
- `s0-s11`: 被调用者保存寄存器（12个）

**为什么只保存callee-saved寄存器？**
- Caller-saved寄存器（a0-a7, t0-t6）由函数调用者负责保存
- 上下文切换发生在函数调用边界，这些寄存器已被保存在栈上

---

### 2. `kernel/process/scheduler.c` - 调度器

```c
static void context_switch(process_t *old, process_t *new)
```

**流程：**
1. 更新进程状态
2. 切换页表（`w_satp`）
3. 调用`switch_context()`切换寄存器

---

### 3. `kernel/process/process.c` - 进程管理

```c
void process_setup_context(process_t *p, void (*entry)(void), void *stack_top)
```

**功能：**初始化新进程的上下文
- 设置`ra`为进程入口点
- 设置`sp`为进程栈顶
- 清零其他寄存器

---

## 使用示例

### 创建并运行新进程

```c
// 1. 分配进程结构
process_t *p = process_alloc();
strcpy(p->name, "test_proc");

// 2. 分配栈空间
void *stack = alloc_page();  // 4KB栈
void *stack_top = (void*)((uint64_t)stack + PAGE_SIZE);

// 3. 设置入口点和栈
void test_process(void) {
    while (1) {
        printf("Process %s running\n", current_proc()->name);
        sched_yield();  // 主动让出CPU
    }
}
process_setup_context(p, test_process, stack_top);

// 4. 添加到调度队列
sched_add(p);

// 5. 调度器会自动切换到该进程
```

---

## 调度算法

**时间片轮转（Round-Robin）：**
- 时间片：10个定时器滴答
- FIFO就绪队列
- 进程用完时间片后自动放回队列尾部

---

## 注意事项

### 1. 栈空间
- 每个进程需要独立的栈
- 推荐大小：至少4KB（一页）
- 必须在创建进程时分配

### 2. 页表
- 如果进程需要独立地址空间，需设置`p->pagetable`
- 内核线程可共享内核页表（设为NULL）

### 3. 进程终止
- 进程函数不应该返回
- 如需终止，调用`process_exit()`（需实现）

### 4. 同步
- 当前实现未考虑并发保护
- 多核系统需要添加自旋锁

---

## 调试技巧

### 检查上下文是否正确保存

```c
void debug_context(context_t *ctx) {
    printf("Context:\n");
    printf("  ra:  %p\n", (void*)ctx->ra);
    printf("  sp:  %p\n", (void*)ctx->sp);
    printf("  s0:  %p\n", (void*)ctx->s0);
    // ...
}
```

### 跟踪进程切换

在`context_switch()`中添加：
```c
printf("[SCHED] Switch: %s (PID %lu) -> %s (PID %lu)\n",
       old ? old->name : "NULL", old ? old->pid : 0,
       new->name, new->pid);
```

---

## 性能特点

- **切换时间：** ~50条指令（14个寄存器 + 页表切换）
- **空间开销：** 112字节/进程（context_t大小）
- **算法复杂度：** O(1)调度

---

## 扩展方向

1. **优先级调度：** 添加优先级队列
2. **进程睡眠/唤醒：** 实现睡眠队列和等待事件
3. **CPU亲和性：** 多核调度优化
4. **实时调度：** 支持实时任务
5. **上下文压缩：** 只保存必要的寄存器
