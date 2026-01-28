# RISC-V 64-bit Embedded OS Build Guide

## Prerequisites

To build and run this project, you need:

1. **RISC-V GNU Toolchain**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install gcc-riscv64-unknown-elf
   
   # Or build from source
   git clone https://github.com/riscv/riscv-gnu-toolchain
   cd riscv-gnu-toolchain
   ./configure --prefix=/opt/riscv --with-arch=rv64imac --with-abi=lp64
   make
   ```

2. **QEMU with RISC-V support**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install qemu-system-riscv64
   
   # Or build from source
   git clone https://github.com/qemu/qemu
   cd qemu
   ./configure --target-list=riscv64-softmmu
   make
   sudo make install
   ```

3. **Build tools**
   ```bash
   sudo apt-get install build-essential make
   ```

## Building the OS

```bash
# Clean previous builds
make clean

# Build the kernel
make all
```

This will generate:
- `build/kernel.elf` - The kernel executable in ELF format
- `build/kernel.bin` - The raw binary kernel
- `build/kernel.asm` - Disassembly listing for debugging

## Running the OS

To run the OS in QEMU:

```bash
make run
```

You should see the kernel boot and present a simple shell prompt:

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

To exit QEMU, press `Ctrl+A` then `X`.

## Debugging

To debug the kernel with GDB:

1. In one terminal, start QEMU in debug mode:
   ```bash
   make debug
   ```

2. In another terminal, start GDB:
   ```bash
   riscv64-unknown-elf-gdb build/kernel.elf
   (gdb) target remote :1234
   (gdb) break kernel_main
   (gdb) continue
   ```

## Shell Commands

The simple shell supports the following commands:

- `help` - Display available commands
- `info` - Show system information
- `test` - Run memory allocation test
- `echo <text>` - Echo text back
- `reboot` - Reboot the system (exit QEMU)

## Project Structure

```
riscv64-embedded-os/
├── bootloader/          # Boot code and entry point
│   └── boot.S          # Assembly boot code
├── kernel/             # Operating system kernel
│   ├── main.c          # Kernel entry and main loop
│   ├── printf.c        # Formatted output
│   ├── types.h         # Type definitions
│   ├── riscv.h         # RISC-V specific functions
│   ├── mm/             # Memory management
│   │   ├── mm.h
│   │   └── mm.c        # Page and heap allocator
│   ├── process/        # Process management
│   │   ├── process.h
│   │   └── process.c   # Process table
│   ├── syscall/        # System calls
│   │   ├── syscall.h
│   │   └── syscall.c   # System call handlers
│   └── trap/           # Interrupt/exception handling
│       ├── trap.h
│       └── trap.c      # Trap handler
├── drivers/            # Hardware drivers
│   ├── uart/           # Serial port driver
│   │   ├── uart.h
│   │   └── uart.c
│   ├── rtc/            # Real-time clock
│   │   ├── rtc.h
│   │   └── rtc.c
│   └── plic/           # Platform interrupt controller
│       ├── plic.h
│       └── plic.c
├── qemu/               # QEMU configuration
│   └── device-tree/    # Device tree files
│       └── virt.dts    # Virtual machine device tree
├── scripts/            # Build scripts
│   └── kernel.ld       # Linker script
├── Makefile            # Main build file
└── README.md           # Project documentation
```

## Features Implemented

### Core Features
- ✅ Boot sequence with stack initialization and BSS clearing
- ✅ UART driver for console I/O
- ✅ Formatted printing (printf)
- ✅ Memory management with page and heap allocators
- ✅ Trap/interrupt handling framework
- ✅ Interactive shell

### Memory Management
- Stack-based page allocator
- Simple bump heap allocator
- Support for 4KB pages
- Memory mapping for kernel space

### I/O
- NS16550A UART driver
- Console input/output
- Simple line editing (backspace support)

### Interrupt Handling
- Trap vector setup
- Exception handling
- Interrupt support framework

## Development Notes

### Memory Layout
- Kernel starts at `0x80000000` (2GB mark)
- 128MB of RAM available
- Stack: 64KB allocated
- Heap: 1MB allocated
- Free pages: Remaining RAM for dynamic allocation

### RISC-V Features Used
- RV64IMAC instruction set
- Supervisor mode (S-mode)
- CSR (Control and Status Registers)
- UART for I/O
- Virtual memory ready (SV39 support in code)

## Troubleshooting

### Build fails with "command not found"
Make sure the RISC-V toolchain is in your PATH:
```bash
export PATH=/opt/riscv/bin:$PATH
```

### QEMU doesn't start
Check that QEMU RISC-V is installed:
```bash
which qemu-system-riscv64
```

### No output in QEMU
Make sure you're using the `-nographic` flag (included in Makefile).

## Future Enhancements

The following features are planned or partially implemented:

- [ ] Full virtual memory with SV39 paging
- [ ] Process scheduling with timer interrupts
- [ ] User mode support
- [ ] ELF loader for user programs
- [ ] File system support
- [ ] Network driver
- [ ] Multi-core support

## License

MIT License - See LICENSE file for details

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.
