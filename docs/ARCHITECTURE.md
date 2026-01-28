# RISC-V 64-bit Embedded OS Architecture

## Overview

This document describes the architecture of the RISC-V 64-bit Embedded OS project.

## System Architecture

```
┌─────────────────────────────────────────┐
│          User Space (U-mode)            │
│  ┌───────────┐  ┌──────────────────┐   │
│  │   Shell   │  │  User Programs   │   │
│  └─────┬─────┘  └────────┬─────────┘   │
│        │                 │              │
│        └─────────┬───────┘              │
└──────────────────┼──────────────────────┘
                   │ System Calls
┌──────────────────┼──────────────────────┐
│                  ▼                       │
│        Kernel Space (S-mode)            │
│  ┌──────────────────────────────────┐   │
│  │     System Call Interface        │   │
│  └────────────┬─────────────────────┘   │
│               │                          │
│  ┌────────────┼─────────────────────┐   │
│  │  Process  Memory   Trap/Int      │   │
│  │  Manager  Manager  Handler       │   │
│  └────────────┼─────────────────────┘   │
│               │                          │
│  ┌────────────┼─────────────────────┐   │
│  │       Device Drivers              │   │
│  │  UART  │  RTC  │  PLIC            │   │
│  └────────────┼─────────────────────┘   │
└───────────────┼──────────────────────────┘
                │ Hardware Access
┌───────────────┼──────────────────────────┐
│               ▼                          │
│        Hardware (M-mode/OpenSBI)        │
│  ┌─────────────────────────────────┐    │
│  │  CPU  │ Memory │ UART │ RTC     │    │
│  └─────────────────────────────────┘    │
└──────────────────────────────────────────┘
```

## Boot Sequence

1. **Power-On / Reset**
   - CPU starts in M-mode at reset vector
   - (In real hardware, M-mode firmware would run)

2. **OpenSBI (M-mode)** - When using QEMU with OpenSBI
   - Initializes hardware
   - Sets up machine mode environment
   - Prepares to jump to S-mode

3. **Bootloader (S-mode)**
   - `boot.S`: Entry point at `_start`
   - Clear interrupts
   - Setup stack pointer
   - Clear BSS section
   - Jump to `kernel_main`

4. **Kernel Initialization**
   - Initialize UART for console
   - Initialize memory manager
   - Initialize trap handlers
   - Start interactive shell

## Memory Layout

### Physical Memory Map (QEMU virt machine)

```
0x00000000 - 0x00000FFF    Debug/Boot ROM
0x00001000 - 0x000FFFFF    (Reserved)
0x00100000 - 0x00100FFF    CLINT (Core Local Interruptor)
0x00101000 - 0x00101FFF    RTC
0x0C000000 - 0x0FFFFFFF    PLIC (Platform Level Interrupt Controller)
0x10000000 - 0x100000FF    UART0 (NS16550A)
0x80000000 - 0x87FFFFFF    RAM (128MB)
```

### Kernel Memory Layout

```
0x80000000                  _start (boot entry)
    |
    ├─ .text.boot           Boot code
    ├─ .text                Kernel code
    ├─ .rodata              Read-only data
    ├─ .data                Initialized data
    ├─ .bss                 Uninitialized data
    |
0x????????                  __stack_start
    ├─ Stack (64KB)         Kernel stack
0x????????                  __stack_top
    |
0x????????                  __heap_start
    ├─ Heap (1MB)           Dynamic allocation
0x????????                  __heap_end
    |
0x????????                  Free pages
    ├─ Page allocator       Remaining RAM
0x87FFFFFF                  End of RAM
```

## Privilege Modes

### Machine Mode (M-mode)
- Highest privilege level
- Full hardware access
- Handled by OpenSBI/firmware
- Not directly used by our kernel

### Supervisor Mode (S-mode)
- Operating system privilege level
- Our kernel runs here
- Can access most CSRs
- Cannot directly access M-mode resources

### User Mode (U-mode)
- Lowest privilege level
- User programs run here (planned)
- Limited access to resources
- Must use system calls for privileged operations

## Module Descriptions

### Bootloader (`bootloader/`)
- **boot.S**: Assembly code for initial boot
  - Sets up stack
  - Clears BSS
  - Jumps to kernel
  - Defines trap entry point

### Kernel Core (`kernel/`)
- **main.c**: Kernel entry point and main loop
  - System initialization
  - Interactive shell
  - Command processing

- **printf.c**: Formatted output
  - printf() implementation
  - panic() for fatal errors

- **types.h**: Type definitions
- **riscv.h**: RISC-V specific inline functions

### Memory Management (`kernel/mm/`)
- **mm.c**: Memory allocator
  - Page allocator (stack-based)
  - Heap allocator (bump allocator)
  - Memory statistics

### Process Management (`kernel/process/`)
- **process.c**: Process table and management
  - Process states
  - Process allocation/deallocation
  - (Scheduling to be implemented)

### System Calls (`kernel/syscall/`)
- **syscall.c**: System call handlers
  - read/write
  - fork/exec (stubs)
  - exit

### Trap Handling (`kernel/trap/`)
- **trap.c**: Interrupt and exception handling
  - Exception handlers
  - Interrupt handlers
  - Context switching (to be implemented)

### Drivers (`drivers/`)
- **uart/**: NS16550A UART driver
  - Character I/O
  - Console support

- **rtc/**: Real-time clock
  - Time reading

- **plic/**: Platform-Level Interrupt Controller
  - Interrupt routing
  - Priority management

## Data Structures

### Process Structure
```c
typedef struct process {
    uint64_t pid;           // Process ID
    proc_state_t state;     // Process state
    uint64_t *pagetable;    // Page table pointer
    uint64_t context;       // Saved context
    uint64_t kernel_sp;     // Kernel stack pointer
    uint64_t user_sp;       // User stack pointer
} process_t;
```

### Page Allocator
- Maintains linked list of free pages
- Each page points to the next free page
- O(1) allocation and deallocation

## Interrupt and Exception Handling

### CSR Registers Used
- `stvec`: Trap vector base address
- `sstatus`: Supervisor status register
- `sie`: Supervisor interrupt enable
- `sip`: Supervisor interrupt pending
- `sepc`: Supervisor exception program counter
- `scause`: Supervisor trap cause
- `stval`: Supervisor trap value
- `sscratch`: Supervisor scratch register

### Trap Flow
1. Exception/interrupt occurs
2. CPU switches to `trap_entry` (set in `stvec`)
3. Save context
4. Call `trap_handler()`
5. Determine trap type (interrupt vs exception)
6. Handle appropriately
7. Restore context
8. Return via `sret`

## Building and Compilation

### Toolchain
- **Compiler**: `riscv64-unknown-elf-gcc`
- **Linker**: `riscv64-unknown-elf-ld`
- **Target**: RV64IMAC (64-bit with integer, multiply, atomic, compressed)
- **ABI**: lp64 (long and pointers are 64-bit)

### Compilation Flags
- `-march=rv64imac`: Target architecture
- `-mabi=lp64`: ABI specification
- `-mcmodel=medany`: Medium any code model
- `-nostdlib`: Don't link standard library
- `-fno-builtin`: Don't use built-in functions

### Linking
- Custom linker script (`scripts/kernel.ld`)
- Sections: `.text.boot`, `.text`, `.rodata`, `.data`, `.bss`
- Stack and heap allocation

## QEMU Configuration

### Virtual Machine
- Machine type: `virt`
- Memory: 128MB
- SMP: 1 core
- Devices: UART, RTC, PLIC, CLINT

### Device Tree
- Located in `qemu/device-tree/virt.dts`
- Describes hardware layout
- Used by OS for device discovery (future)

## Testing

### Manual Testing
1. Build kernel: `make all`
2. Run in QEMU: `make run`
3. Test shell commands:
   - `help`: List commands
   - `info`: Show system info
   - `test`: Test memory allocation
   - `echo hello`: Echo text
   - `reboot`: Exit QEMU

### Memory Test
- Allocates pages
- Allocates heap memory
- Verifies allocations
- Frees pages
- Checks for leaks

## Future Enhancements

### Virtual Memory (SV39)
- 3-level page tables
- 512GB virtual address space
- User/kernel separation

### Scheduling
- Round-robin scheduler
- Timer-based preemption
- Process context switching

### User Space
- ELF loader
- User mode execution
- System call interface

### File System
- Simple file system
- VFS layer
- Device files

## References

- [RISC-V Specifications](https://riscv.org/technical/specifications/)
- [RISC-V Privileged Architecture](https://github.com/riscv/riscv-isa-manual)
- [QEMU RISC-V Documentation](https://www.qemu.org/docs/master/system/target-riscv.html)
- [xv6-riscv](https://github.com/mit-pdos/xv6-riscv) - Reference OS implementation
