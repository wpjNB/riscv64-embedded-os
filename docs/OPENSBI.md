# OpenSBI Reference

This project uses QEMU's built-in OpenSBI firmware. When running with `make run`, QEMU automatically loads OpenSBI as the bootloader, which then loads our kernel.

## About OpenSBI

OpenSBI (Open Source Supervisor Binary Interface) is a RISC-V bootloader that:
- Initializes the hardware
- Sets up M-mode (Machine mode)
- Provides runtime services to the OS in S-mode (Supervisor mode)
- Loads and jumps to the kernel

## QEMU Integration

QEMU virt machine includes OpenSBI by default when using:
```bash
qemu-system-riscv64 -machine virt -bios default
```

Our Makefile uses `-bios none` to skip OpenSBI and boot the kernel directly at 0x80000000.

## SBI Calls

The kernel can make SBI calls to OpenSBI for hardware services:

- Console output: `SBI_CONSOLE_PUTCHAR`
- Console input: `SBI_CONSOLE_GETCHAR`
- Timer: `SBI_SET_TIMER`
- IPI: `SBI_SEND_IPI`
- Remote fence: `SBI_REMOTE_FENCE_I`, `SBI_REMOTE_SFENCE_VMA`
- Shutdown: `SBI_SHUTDOWN`

## Custom OpenSBI Build (Future)

To use a custom OpenSBI build:

```bash
git clone https://github.com/riscv-software-src/opensbi.git
cd opensbi
make PLATFORM=generic CROSS_COMPILE=riscv64-unknown-elf-
```

Then use it with QEMU:
```bash
qemu-system-riscv64 -machine virt -bios build/platform/generic/firmware/fw_jump.bin \
    -kernel build/kernel.bin
```

## References

- [OpenSBI Documentation](https://github.com/riscv-software-src/opensbi/tree/master/docs)
- [RISC-V SBI Specification](https://github.com/riscv-non-isa/riscv-sbi-doc)
