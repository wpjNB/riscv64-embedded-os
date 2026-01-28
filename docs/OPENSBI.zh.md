# OpenSBI 参考

本项目使用 QEMU 内置的 OpenSBI 固件。使用 `make run` 运行时，QEMU 会自动加载 OpenSBI 作为 bootloader，然后加载我们的内核。

## 关于 OpenSBI

OpenSBI（开源监管者二进制接口）是一个 RISC-V bootloader，它：
- 初始化硬件
- 设置 M-mode（机器模式）
- 为 S-mode（监管者模式）中的操作系统提供运行时服务
- 加载并跳转到内核

## QEMU 集成

QEMU virt 机器在使用以下命令时默认包含 OpenSBI：
```bash
qemu-system-riscv64 -machine virt -bios default
```

我们的 Makefile 使用 `-bios none` 跳过 OpenSBI，直接在 0x80000000 启动内核。

## SBI 调用

内核可以向 OpenSBI 发出 SBI 调用以获取硬件服务：

- 控制台输出：`SBI_CONSOLE_PUTCHAR`
- 控制台输入：`SBI_CONSOLE_GETCHAR`
- 定时器：`SBI_SET_TIMER`
- IPI：`SBI_SEND_IPI`
- 远程栅栏：`SBI_REMOTE_FENCE_I`、`SBI_REMOTE_SFENCE_VMA`
- 关机：`SBI_SHUTDOWN`

## 自定义 OpenSBI 构建（未来）

使用自定义 OpenSBI 构建：

```bash
git clone https://github.com/riscv-software-src/opensbi.git
cd opensbi
make PLATFORM=generic CROSS_COMPILE=riscv64-unknown-elf-
```

然后在 QEMU 中使用：
```bash
qemu-system-riscv64 -machine virt -bios build/platform/generic/firmware/fw_jump.bin \
    -kernel build/kernel.bin
```

## 参考资料

- [OpenSBI 文档](https://github.com/riscv-software-src/opensbi/tree/master/docs)
- [RISC-V SBI 规范](https://github.com/riscv-non-isa/riscv-sbi-doc)
