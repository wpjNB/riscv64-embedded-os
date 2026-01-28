# RISCV64 Embedded OS

> 基于 Qemu 创建嵌入式 RISCV64 操作系统

## 📋 项目简介

本项目是 [rCore](https://github.com/rcore-os/rCore) 的 C 语言重写版，在 Qemu 虚拟机上自定义了一块 RISCV64 的 SOC，并基于此虚拟硬件编写了一个简单操作系统内核。

**项目时间**: 2026.1 - 

## 🛠️ 技术栈

- **语言**: C, Rust
- **虚拟化**: Qemu
- **固件**: OpenSBI
- **架构**: RISC-V (RV64)

## ✨ 核心功能

### 硬件层面
- 使用 Qemu 的内置接口定义了一块 RV64 的 SOC，包含以下硬件：
  - Flash 存储器
  - PLIC (Platform-Level Interrupt Controller)
  - CLINT (Core Local Interruptor)
  - RTC (Real-Time Clock)
  - UART (串口通信)

### 启动流程
- SOC 编写设备树文件
- 移植 OpenSBI 作为 bootloader
- 编写 Boot 代码在 Flash 运行并引导加载内核到 DRAM 执行

### 内存管理
- 使用栈式数据结构管理和分配内存
- 基于 RTC 时钟中断实现了分时多任务调度机制

### 虚拟内存
- 基于 RISCV 的 SV39 分页机制实现了运行时进程虚拟地址空间的映射
- 内核与用户态使用不同的映射方式

### 系统功能
- 实现了一个应用加载机制，通过万能函数二进制文件，生成独立进程的 ELF 编号
- 实现了常用系统调用接口：
  - `fork` - 进程创建
  - `exec` - 程序执行
  - `read` - 读取操作
  - `write` - 写入操作
  - 在系统启动时会拉起 `user_shell` 进程

### 安全性
- 对 CPU 核心的内存屏障进行了安全域与非安全域的分隔
- 在安全域移植了 FreeRTOS 运行

## 📁 项目结构

```
riscv64-embedded-os/
├── bootloader/          # Boot 引导代码
├── kernel/              # 操作系统内核
│   ├── mm/             # 内存管理
│   ├── process/        # 进程管理
│   ├── syscall/        # 系统调用
│   └── trap/           # 中断处理
├── user/               # 用户态程序
│   └── shell/          # 用户 Shell
├── drivers/            # 硬件驱动
│   ├── uart/           # 串口驱动
│   ├── rtc/            # RTC 驱动
│   └── plic/           # 中断控制器
├── opensbi/            # OpenSBI 移植
├── qemu/               # Qemu 配置
│   └── device-tree/    # 设备树文件
├── scripts/            # 构建脚本
├── Makefile            # 主 Makefile
└── README.md           # 项目文档
```

## 🚀 快速开始

### 环境要求

- RISC-V 工具链 (riscv64-unknown-elf-gcc)
- Qemu (支持 RISC-V)
- Make

### 自动安装环境

```bash
# 运行自动安装脚本（Linux/macOS）
./scripts/setup.sh
```

### 编译

```bash
make all
```

### 运行

```bash
make run
```

### 调试

```bash
make debug
```

## 📖 文档

详细文档请查看 `docs/` 目录：

### 中文文档
- [构建指南 (BUILD.zh.md)](docs/BUILD.zh.md) - 如何编译和运行项目
- [开发指南 (DEVELOPMENT.zh.md)](docs/DEVELOPMENT.zh.md) - 开发、调试和贡献指南
- [系统架构 (ARCHITECTURE.zh.md)](docs/ARCHITECTURE.zh.md) - 系统架构和设计文档
- [OpenSBI 参考 (OPENSBI.zh.md)](docs/OPENSBI.zh.md) - OpenSBI 引导程序说明

### English Documentation
- [Build Guide (BUILD.md)](docs/BUILD.md) - How to compile and run the project
- [Development Guide (DEVELOPMENT.md)](docs/DEVELOPMENT.md) - Development, debugging and contribution guide
- [System Architecture (ARCHITECTURE.md)](docs/ARCHITECTURE.md) - System architecture and design documentation
- [OpenSBI Reference (OPENSBI.md)](docs/OPENSBI.md) - OpenSBI bootloader documentation

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

## 📄 许可证

MIT License

## 👤 作者

[@wpjNB](https://github.com/wpjNB)

---

⭐ 如果这个项目对你有帮助，欢迎 Star！
