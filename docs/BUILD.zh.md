# RISC-V 64 位嵌入式操作系统构建指南

## 前置要求

要构建和运行本项目，你需要：

1. **RISC-V GNU 工具链**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install gcc-riscv64-unknown-elf
   
   # 或从源码构建
   git clone https://github.com/riscv/riscv-gnu-toolchain
   cd riscv-gnu-toolchain
   ./configure --prefix=/opt/riscv --with-arch=rv64imac --with-abi=lp64
   make
   ```

2. **QEMU（支持 RISC-V）**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install qemu-system-riscv64
   
   # 或从源码构建
   git clone https://github.com/qemu/qemu
   cd qemu
   ./configure --target-list=riscv64-softmmu
   make
   sudo make install
   ```

3. **构建工具**
   ```bash
   sudo apt-get install build-essential make
   ```

## 构建操作系统

```bash
# 清理之前的构建
make clean

# 构建内核
make all
```

这将生成：
- `build/kernel.elf` - ELF 格式的内核可执行文件
- `build/kernel.bin` - 原始二进制内核
- `build/kernel.asm` - 用于调试的反汇编列表

## 运行操作系统

在 QEMU 中运行操作系统：

```bash
make run
```

你将看到内核启动并显示一个简单的 shell 提示符：

```
====================================
  RISC-V 64-bit Embedded OS
  Version 1.0
====================================

[KERNEL] Starting RISC-V OS kernel...
[KERNEL] Kernel loaded at 0x80000000
[MM] Initializing memory manager
...
[SHELL] Starting simple shell
Type 'help' for available commands
>
```

要退出 QEMU，按 `Ctrl+A` 然后按 `X`。

## 调试

使用 GDB 调试内核：

1. 在一个终端中，以调试模式启动 QEMU：
   ```bash
   make debug
   ```

2. 在另一个终端中，启动 GDB：
   ```bash
   riscv64-unknown-elf-gdb build/kernel.elf
   (gdb) target remote :1234
   (gdb) break kernel_main
   (gdb) continue
   ```

## Shell 命令

简单的 shell 支持以下命令：

- `help` - 显示可用命令
- `info` - 显示系统信息
- `test` - 运行内存分配测试
- `echo <文本>` - 回显文本
- `reboot` - 重启系统（退出 QEMU）

## 项目结构

```
riscv64-embedded-os/
├── bootloader/          # 启动代码和入口点
│   └── boot.S          # 汇编启动代码
├── kernel/             # 操作系统内核
│   ├── main.c          # 内核入口和主循环
│   ├── printf.c        # 格式化输出
│   ├── types.h         # 类型定义
│   ├── riscv.h         # RISC-V 特定函数
│   ├── mm/             # 内存管理
│   │   ├── mm.h
│   │   └── mm.c        # 页和堆分配器
│   ├── process/        # 进程管理
│   │   ├── process.h
│   │   └── process.c   # 进程表
│   ├── syscall/        # 系统调用
│   │   ├── syscall.h
│   │   └── syscall.c   # 系统调用处理器
│   └── trap/           # 中断/异常处理
│       ├── trap.h
│       └── trap.c      # 陷阱处理器
├── drivers/            # 硬件驱动
│   ├── uart/           # 串口驱动
│   │   ├── uart.h
│   │   └── uart.c
│   ├── rtc/            # 实时时钟
│   │   ├── rtc.h
│   │   └── rtc.c
│   └── plic/           # 平台中断控制器
│       ├── plic.h
│       └── plic.c
├── qemu/               # QEMU 配置
│   └── device-tree/    # 设备树文件
│       └── virt.dts    # 虚拟机设备树
├── scripts/            # 构建脚本
│   └── kernel.ld       # 链接器脚本
├── Makefile            # 主构建文件
└── README.md           # 项目文档
```

## 已实现功能

### 核心功能
- ✅ 启动序列，包含栈初始化和 BSS 清零
- ✅ 用于控制台 I/O 的 UART 驱动
- ✅ 格式化打印（printf）
- ✅ 内存管理，包含页和堆分配器
- ✅ 陷阱/中断处理框架
- ✅ 交互式 shell

### 内存管理
- 基于栈的页分配器
- 简单的堆分配器
- 支持 4KB 页
- 内核空间内存映射

### I/O
- NS16550A UART 驱动
- 控制台输入/输出
- 简单的行编辑（支持退格）

### 中断处理
- 陷阱向量设置
- 异常处理
- 中断支持框架

## 开发注意事项

### 内存布局
- 内核从 `0x80000000`（2GB 标记）开始
- 可用 128MB RAM
- 栈：分配 64KB
- 堆：分配 1MB
- 空闲页：剩余 RAM 用于动态分配

### 使用的 RISC-V 功能
- RV64IMAC 指令集
- 监管者模式（S-mode）
- CSR（控制和状态寄存器）
- UART 用于 I/O
- 虚拟内存就绪（代码中支持 SV39）

## 故障排除

### 构建失败，提示"command not found"
确保 RISC-V 工具链在你的 PATH 中：
```bash
export PATH=/opt/riscv/bin:$PATH
```

### QEMU 无法启动
检查是否安装了 QEMU RISC-V：
```bash
which qemu-system-riscv64
```

### QEMU 中没有输出
确保使用了 `-nographic` 标志（包含在 Makefile 中）。

## 未来增强功能

以下功能已计划或部分实现：

- [ ] 完整的 SV39 分页虚拟内存
- [ ] 基于定时器中断的进程调度
- [ ] 用户模式支持
- [ ] 用户程序的 ELF 加载器
- [ ] 文件系统支持
- [ ] 网络驱动
- [ ] 多核支持

## 许可证

MIT License - 详见 LICENSE 文件

## 贡献

欢迎贡献！请随时提交 issue 或 pull request。
