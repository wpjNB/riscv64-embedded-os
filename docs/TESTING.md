# 测试和验证指南 / Testing and Validation Guide

本文档描述了如何测试和验证新实现的功能。
This document describes how to test and validate the newly implemented features.

## 编译项目 / Building the Project

### 先决条件 / Prerequisites
```bash
# 安装 RISC-V 工具链 / Install RISC-V toolchain
# 参见 scripts/setup.sh / See scripts/setup.sh

# 或使用 Docker 容器 / Or use Docker container
docker run -it --rm -v $(pwd):/work riscv/riscv-gnu-toolchain
```

### 编译 / Build
```bash
make clean
make all
```

### 运行 / Run
```bash
make run
```

## 测试清单 / Test Checklist

### 1. 虚拟内存测试 / Virtual Memory Tests

#### 测试项 / Test Items
- [ ] 页表初始化成功 / Page table initialization successful
- [ ] 内核映射正确 / Kernel mapping correct
- [ ] 设备映射正确 / Device mapping correct
- [ ] 用户页表创建成功 / User page table creation successful
- [ ] 页表行走功能正常 / Page table walking works
- [ ] 地址转换正确 / Address translation correct
- [ ] TLB 刷新正常 / TLB flush works

#### 预期输出 / Expected Output
```
[VM] Initializing SV39 virtual memory
[VM] Created kernel page table at 0x...
[VM] Kernel page table initialized
[VM] Switched to SV39 paging mode
```

#### 验证方法 / Verification Method
```c
// 在 kernel/main.c 中添加测试代码
void test_vm(void) {
    printf("[TEST] Testing virtual memory...\n");
    
    // 测试用户页表创建
    pagetable_t pt = vm_create_user_pagetable();
    assert(pt != NULL);
    
    // 测试映射
    uint64_t pa = 0x80100000;
    int ret = mappages(pt, 0x1000, PAGE_SIZE, pa, PTE_R | PTE_W | PTE_U);
    assert(ret == 0);
    
    // 测试地址转换
    uint64_t pa2 = walkaddr(pt, 0x1000);
    assert(pa2 == pa);
    
    printf("[TEST] Virtual memory test passed\n");
}
```

### 2. 调度器测试 / Scheduler Tests

#### 测试项 / Test Items
- [ ] 调度器初始化成功 / Scheduler initialization successful
- [ ] 进程可以添加到就绪队列 / Processes can be added to ready queue
- [ ] 轮转调度正常工作 / Round-robin scheduling works
- [ ] 时间片抢占正常 / Time-slice preemption works
- [ ] 上下文切换正常 / Context switching works

#### 预期输出 / Expected Output
```
[SCHED] Initializing round-robin scheduler
[SCHED] Time slice: 10 ticks
[SCHED] Starting scheduler
```

#### 验证方法 / Verification Method
```c
void test_scheduler(void) {
    printf("[TEST] Testing scheduler...\n");
    
    // 创建测试进程
    process_t *p1 = process_alloc();
    strcpy(p1->name, "test1");
    
    process_t *p2 = process_alloc();
    strcpy(p2->name, "test2");
    
    // 添加到调度器
    sched_add(p1);
    sched_add(p2);
    
    // 测试调度
    sched_yield();
    
    printf("[TEST] Scheduler test passed\n");
}
```

### 3. ELF 加载器测试 / ELF Loader Tests

#### 测试项 / Test Items
- [ ] ELF 头验证正常 / ELF header validation works
- [ ] 程序头解析正确 / Program header parsing correct
- [ ] 段加载正常 / Segment loading works
- [ ] 入口点识别正确 / Entry point identification correct

#### 测试 ELF 文件 / Test ELF File
创建一个简单的测试程序：
Create a simple test program:

```c
// user/test.c
void _start(void) {
    // 简单的用户程序
    // Simple user program
    while(1) {
        // Do something
    }
}
```

编译：/ Compile:
```bash
riscv64-unknown-elf-gcc -march=rv64imac -mabi=lp64 -nostdlib -Ttext=0x1000 -o test.elf user/test.c
```

#### 验证方法 / Verification Method
```c
void test_elf_loader(void) {
    printf("[TEST] Testing ELF loader...\n");
    
    // 加载 ELF 文件 (假设已读入内存)
    // Load ELF file (assuming already in memory)
    extern uint8_t _binary_test_elf_start[];
    extern uint8_t _binary_test_elf_end[];
    
    uint64_t entry;
    int ret = elf_load(_binary_test_elf_start, 
                      _binary_test_elf_end - _binary_test_elf_start,
                      &entry);
    
    assert(ret == 0);
    printf("[TEST] ELF entry point: %p\n", (void*)entry);
    printf("[TEST] ELF loader test passed\n");
}
```

### 4. 文件系统测试 / File System Tests

#### 测试项 / Test Items
- [ ] VFS 初始化成功 / VFS initialization successful
- [ ] SimpleFS 格式化成功 / SimpleFS format successful
- [ ] 文件创建成功 / File creation successful
- [ ] 文件删除成功 / File deletion successful
- [ ] 设备注册成功 / Device registration successful

#### 预期输出 / Expected Output
```
[VFS] Initializing Virtual File System
[VFS] VFS initialized
[SFS] Initializing Simple File System
[SFS] Formatting file system with 256 blocks
[SFS] File system formatted successfully
```

#### 验证方法 / Verification Method
```c
void test_filesystem(void) {
    printf("[TEST] Testing file system...\n");
    
    // 测试文件创建
    int ino = sfs_create("testfile", VFS_FILE);
    assert(ino > 0);
    printf("[TEST] Created file with inode %d\n", ino);
    
    // 测试文件删除
    int ret = sfs_delete("testfile");
    assert(ret == 0);
    
    printf("[TEST] File system test passed\n");
}
```

### 5. 系统调用测试 / System Call Tests

#### 测试项 / Test Items
- [ ] SYS_GETPID 返回正确的 PID / SYS_GETPID returns correct PID
- [ ] SYS_YIELD 正常工作 / SYS_YIELD works
- [ ] SYS_OPEN/CLOSE 正常工作 / SYS_OPEN/CLOSE work
- [ ] SYS_READ/WRITE 正常工作 / SYS_READ/WRITE work

#### 验证方法 / Verification Method
```c
void test_syscalls(void) {
    printf("[TEST] Testing system calls...\n");
    
    // 测试 getpid
    uint64_t pid = syscall_handler(SYS_GETPID, 0, 0, 0);
    printf("[TEST] Current PID: %lu\n", pid);
    
    // 测试 yield
    syscall_handler(SYS_YIELD, 0, 0, 0);
    
    printf("[TEST] System call test passed\n");
}
```

## 集成测试 / Integration Tests

### 完整系统测试 / Full System Test

在 `kernel/main.c` 中添加：
Add to `kernel/main.c`:

```c
static void run_tests(void) {
    printf("\n[TEST] Starting system tests...\n\n");
    
    test_vm();
    test_scheduler();
    test_elf_loader();
    test_filesystem();
    test_syscalls();
    
    printf("\n[TEST] All tests passed!\n\n");
}

void kernel_main(void) {
    // ... 现有初始化代码 ...
    // ... existing initialization code ...
    
    /* Run tests */
    run_tests();
    
    /* Start shell */
    run_shell();
}
```

## 性能测试 / Performance Tests

### 内存分配性能 / Memory Allocation Performance
```c
void test_memory_performance(void) {
    uint64_t start = read_time();
    
    for (int i = 0; i < 1000; i++) {
        void *page = alloc_page();
        free_page(page);
    }
    
    uint64_t end = read_time();
    printf("[PERF] 1000 page alloc/free: %lu cycles\n", end - start);
}
```

### 上下文切换性能 / Context Switch Performance
```c
void test_context_switch_performance(void) {
    uint64_t start = read_time();
    
    for (int i = 0; i < 1000; i++) {
        sched_yield();
    }
    
    uint64_t end = read_time();
    printf("[PERF] 1000 context switches: %lu cycles\n", end - start);
}
```

## 调试技巧 / Debugging Tips

### GDB 调试 / GDB Debugging
```bash
# 终端 1 / Terminal 1
make debug

# 终端 2 / Terminal 2
riscv64-unknown-elf-gdb build/kernel.elf
(gdb) target remote :1234
(gdb) b kernel_main
(gdb) c
```

### 打印调试信息 / Print Debug Information
在代码中添加：/ Add to code:
```c
#define DEBUG 1
#ifdef DEBUG
#define debug_printf(...) printf(__VA_ARGS__)
#else
#define debug_printf(...)
#endif
```

### 检查页表 / Inspect Page Tables
```c
void dump_pagetable(pagetable_t pt, int level) {
    for (int i = 0; i < 512; i++) {
        pte_t pte = pt[i];
        if (PTE_VALID(pte)) {
            printf("[PT L%d][%d] PTE: %p -> PA: %p flags: %lx\n",
                   level, i, (void*)pte, (void*)PTE2PA(pte), 
                   PTE_FLAGS(pte));
        }
    }
}
```

## 故障排除 / Troubleshooting

### 常见问题 / Common Issues

#### 1. 编译错误 / Compilation Errors
```
错误: undefined reference to `xxx'
解决: 检查 Makefile 是否包含所有源文件
Error: undefined reference to `xxx'
Solution: Check if Makefile includes all source files
```

#### 2. 运行时崩溃 / Runtime Crashes
```
症状: 内核在初始化时崩溃
检查:
1. 页表映射是否正确
2. 栈大小是否足够
3. BSS 段是否正确清零

Symptom: Kernel crashes during initialization
Check:
1. Page table mappings are correct
2. Stack size is sufficient
3. BSS segment is properly zeroed
```

#### 3. QEMU 无法启动 / QEMU Won't Start
```
错误: qemu-system-riscv64: command not found
解决: 安装 QEMU 或检查 PATH

Error: qemu-system-riscv64: command not found
Solution: Install QEMU or check PATH
```

## 测试报告模板 / Test Report Template

```
测试日期 / Test Date: YYYY-MM-DD
测试人员 / Tester: XXX
提交哈希 / Commit Hash: xxxxxxx

测试环境 / Test Environment:
- QEMU 版本 / QEMU Version: x.x.x
- GCC 版本 / GCC Version: x.x.x
- 操作系统 / Host OS: Linux/macOS/Windows

测试结果 / Test Results:
✓ 虚拟内存 / Virtual Memory: PASS
✓ 调度器 / Scheduler: PASS
✓ ELF 加载器 / ELF Loader: PASS
✓ 文件系统 / File System: PASS
✓ 系统调用 / System Calls: PASS

性能指标 / Performance Metrics:
- 页分配延迟 / Page Alloc Latency: XXX cycles
- 上下文切换延迟 / Context Switch Latency: XXX cycles

问题和建议 / Issues and Suggestions:
1. ...
2. ...
```

## 持续集成 / Continuous Integration

建议设置 CI 流程：/ Recommended CI setup:

```yaml
# .github/workflows/ci.yml
name: CI

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install RISC-V toolchain
        run: sudo apt-get install gcc-riscv64-unknown-elf
      - name: Build
        run: make all
      - name: Run tests (QEMU)
        run: timeout 30s make run || true
```

## 总结 / Summary

完成所有测试后，确保：
After completing all tests, ensure:

- [ ] 所有核心功能都经过测试 / All core features are tested
- [ ] 文档与代码同步 / Documentation matches code
- [ ] 性能满足预期 / Performance meets expectations
- [ ] 没有内存泄漏 / No memory leaks
- [ ] 代码风格一致 / Code style is consistent
