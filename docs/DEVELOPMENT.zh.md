# RISC-V 64 位嵌入式操作系统 - 开发指南

## 项目状态

这是一个完整实现的最小化 RISC-V 64 位嵌入式操作系统，包括：

✅ **已实现功能：**
- 完整的 bootloader，正确初始化
- 内存管理（页分配器 + 堆分配器）
- 用于控制台 I/O 的 UART 驱动
- 陷阱和中断处理框架
- 进程管理结构
- 系统调用接口（read、write、fork/exec 桩函数）
- 带多个命令的交互式 shell
- 全面的文档

## 代码组织

### 需要理解的关键文件

1. **入口点**：`bootloader/boot.S`
   - 首先运行的汇编代码
   - 设置栈，清除 BSS，跳转到内核

2. **内核主函数**：`kernel/main.c`
   - 主内核初始化
   - 系统组件初始化
   - 交互式 shell 实现

3. **内存管理器**：`kernel/mm/mm.c`
   - 页分配（基于链表）
   - 堆分配（bump 分配器）

4. **UART 驱动**：`drivers/uart/uart.c`
   - 低级 UART I/O
   - 与 QEMU NS16550A 配合使用

5. **Printf**：`kernel/printf.c`
   - 格式化输出实现
   - 支持 %d、%x、%s、%p、%c

6. **陷阱处理器**：`kernel/trap/trap.c`
   - 异常和中断处理
   - CSR 寄存器管理

## 构建系统

Makefile 配置为：
- 使用 `riscv64-unknown-elf-gcc` 进行交叉编译
- 目标：RV64IMAC（64 位，带 M、A、C 扩展）
- ABI：lp64
- 输出：ELF 可执行文件和原始二进制文件

### 构建目标

```bash
make all        # 构建 kernel.elf 和 kernel.bin
make run        # 在 QEMU 中运行
make debug      # 在 QEMU 中运行，支持 GDB
make clean      # 清理构建产物
```

## 内存管理详情

### 页分配器
- 使用空闲页的栈（链表）
- 每个 4KB 页包含指向下一个空闲页的指针
- O(1) 分配和释放
- 分配时页会被零初始化

### 堆分配器
- 用于小对象的简单 bump 分配器
- 在链接器脚本中保留 1MB 堆
- 快速分配，无释放
- 用于内核数据结构

### 内存映射
```
0x80000000 - 内核开始
    ├─ 启动代码
    ├─ 内核代码
    ├─ 数据
    ├─ BSS
    ├─ 栈 (64KB)
    ├─ 堆 (1MB)
    └─ 空闲页（剩余 RAM）
0x87FFFFFF - 128MB RAM 结束
```

## 中断处理

### 支持的陷阱
- 软件中断（SIE）
- 定时器中断（TIE）
- 外部中断（EIE）
- 各种异常（页错误、非法指令等）

### 陷阱流程
```
异常/中断
    ↓
trap_entry (boot.S)
    ↓
trap_handler (trap.c)
    ↓
处理特定陷阱
    ↓
返回 (sret)
```

## Shell 命令

交互式 shell 支持：

- `help` - 显示可用命令
- `info` - 显示详细系统信息
- `test` - 运行内存分配测试
- `echo <文本>` - 向控制台回显文本
- `reboot` - 退出 QEMU（模拟重启）

## 添加新功能

### 添加新驱动

1. 创建目录：`drivers/mydriver/`
2. 添加头文件：`drivers/mydriver/mydriver.h`
3. 添加实现：`drivers/mydriver/mydriver.c`
4. 在内核中包含：`#include "../drivers/mydriver/mydriver.h"`
5. 更新 Makefile 以包含新源文件

### 添加新系统调用

1. 在 `kernel/syscall/syscall.h` 中定义系统调用号：
   ```c
   #define SYS_MYNEWCALL 10
   ```

2. 在 `kernel/syscall/syscall.c` 中添加处理器：
   ```c
   case SYS_MYNEWCALL: {
       // 处理系统调用
       return 0;
   }
   ```

3. 从用户代码使用 `ecall` 指令调用

### 添加新 Shell 命令

在 `kernel/main.c` 的 `run_shell()` 函数中添加：

```c
else if (/* 比较命令 */) {
    // 处理命令
}
```

## 代码风格

- **缩进**：4 个空格
- **大括号**：K&R 风格（开括号在同一行）
- **命名**：函数和变量使用 snake_case
- **类型**：使用 stdint 类型（uint64_t 等）
- **注释**：多行使用 C 风格 /* */，单行使用 //

## 测试策略

### 单元测试（手动）
- 内存分配：`test` 命令
- UART I/O：任何 shell 命令
- Printf：各种格式说明符
- 陷阱处理：可以用非法指令触发

### 集成测试
- 启动序列：观察内核启动消息
- Shell 交互：尝试所有命令
- 内存管理：多次分配/释放页

### 调试技巧

1. **使用 GDB**：
   ```bash
   # 终端 1
   make debug
   
   # 终端 2
   riscv64-unknown-elf-gdb build/kernel.elf
   (gdb) target remote :1234
   (gdb) break kernel_main
   (gdb) continue
   ```

2. **添加调试打印**：
   ```c
   printf("[DEBUG] Variable x = %d\n", x);
   ```

3. **检查内存**：
   ```c
   printf("[MEM] Address %p = %x\n", ptr, *(uint64_t*)ptr);
   ```

## 常见问题和解决方案

### 问题：内核无法启动
- 检查链接器脚本地址是否与 QEMU 内存映射匹配
- 验证 boot.S 是否正确初始化栈
- 确保所有段正确对齐

### 问题：随机崩溃
- 检查栈溢出（在链接器脚本中增加栈大小）
- 验证指针在解引用前的有效性
- 查找字符串操作中的缓冲区溢出

### 问题：UART 上没有输出
- 验证 UART 基地址（QEMU virt 为 0x10000000）
- 检查是否调用了 uart_init()
- 确保 QEMU 使用 `-nographic` 标志运行

## 性能考虑

### 内存分配
- 页分配：O(1)
- 堆分配：O(1)
- 如果需要许多小分配，考虑堆的空闲列表

### I/O
- UART 是轮询（阻塞）的，考虑中断驱动的 I/O
- 在发送前缓冲多个字符

### 上下文切换
- 保存/恢复最小上下文
- 使用 sscratch 快速访问上下文指针

## 安全考虑

### 当前状态
- 无特权分离（所有代码在 S-mode 运行）
- 无内存保护（虚拟内存未启用）
- 无输入验证

### 未来改进
- 启用 SV39 分页以实现内存保护
- 为应用程序添加用户模式
- 验证所有用户输入
- 实现栈保护

## 资源

### RISC-V 文档
- [RISC-V 规范](https://riscv.org/technical/specifications/)
- [RISC-V 汇编程序员手册](https://github.com/riscv-non-isa/riscv-asm-manual)
- [RISC-V 特权 ISA](https://github.com/riscv/riscv-isa-manual)

### 类似项目
- [xv6-riscv](https://github.com/mit-pdos/xv6-riscv) - MIT 的教学操作系统
- [rCore](https://github.com/rcore-os/rCore) - Rust 操作系统（本项目的灵感来源）

### QEMU 文档
- [QEMU RISC-V](https://www.qemu.org/docs/master/system/target-riscv.html)
- [QEMU virt 机器](https://www.qemu.org/docs/master/system/riscv/virt.html)

## 贡献

贡献时：
1. 遵循现有代码风格
2. 为复杂逻辑添加注释
3. 彻底测试
4. 更新文档
5. 保持更改最小且集中

## 许可证

MIT License - 详见 LICENSE 文件
