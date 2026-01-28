# RISC-V 64-bit Embedded OS - Development Guide

## Project Status

This is a complete implementation of a minimal RISC-V 64-bit embedded operating system that includes:

✅ **Implemented Features:**
- Complete bootloader with proper initialization
- Memory management (page allocator + heap allocator)
- UART driver for console I/O
- Trap and interrupt handling framework
- Process management structure
- System call interface (read, write, fork/exec stubs)
- Interactive shell with multiple commands
- Comprehensive documentation

## Code Organization

### Key Files to Understand

1. **Entry Point**: `bootloader/boot.S`
   - Assembly code that runs first
   - Sets up stack, clears BSS, jumps to kernel

2. **Kernel Main**: `kernel/main.c`
   - Main kernel initialization
   - System component initialization
   - Interactive shell implementation

3. **Memory Manager**: `kernel/mm/mm.c`
   - Page allocation (linked-list based)
   - Heap allocation (bump allocator)

4. **UART Driver**: `drivers/uart/uart.c`
   - Low-level UART I/O
   - Works with QEMU NS16550A

5. **Printf**: `kernel/printf.c`
   - Formatted output implementation
   - Supports %d, %x, %s, %p, %c

6. **Trap Handler**: `kernel/trap/trap.c`
   - Exception and interrupt handling
   - CSR register management

## Build System

The Makefile is configured for:
- Cross-compilation with `riscv64-unknown-elf-gcc`
- Target: RV64IMAC (64-bit with M, A, C extensions)
- ABI: lp64
- Output: ELF executable and raw binary

### Build Targets

```bash
make all        # Build kernel.elf and kernel.bin
make run        # Run in QEMU
make debug      # Run in QEMU with GDB support
make clean      # Clean build artifacts
```

## Memory Management Details

### Page Allocator
- Uses a stack (linked-list) of free pages
- Each 4KB page contains pointer to next free page
- O(1) allocation and deallocation
- Pages are zero-initialized on allocation

### Heap Allocator
- Simple bump allocator for small objects
- 1MB heap reserved in linker script
- Fast allocation, no deallocation
- Used for kernel data structures

### Memory Map
```
0x80000000 - Kernel start
    ├─ Boot code
    ├─ Kernel code
    ├─ Data
    ├─ BSS
    ├─ Stack (64KB)
    ├─ Heap (1MB)
    └─ Free pages (remaining RAM)
0x87FFFFFF - End of 128MB RAM
```

## Interrupt Handling

### Supported Traps
- Software interrupts (SIE)
- Timer interrupts (TIE)
- External interrupts (EIE)
- Various exceptions (page fault, illegal instruction, etc.)

### Trap Flow
```
Exception/Interrupt
    ↓
trap_entry (boot.S)
    ↓
trap_handler (trap.c)
    ↓
Handle specific trap
    ↓
Return (sret)
```

## Shell Commands

The interactive shell supports:

- `help` - Display available commands
- `info` - Show detailed system information
- `test` - Run memory allocation test
- `echo <text>` - Echo text back to console
- `reboot` - Exit QEMU (simulated reboot)

## Adding New Features

### Adding a New Driver

1. Create directory: `drivers/mydriver/`
2. Add header: `drivers/mydriver/mydriver.h`
3. Add implementation: `drivers/mydriver/mydriver.c`
4. Include in kernel: `#include "../drivers/mydriver/mydriver.h"`
5. Update Makefile to include new source files

### Adding a New System Call

1. Define syscall number in `kernel/syscall/syscall.h`:
   ```c
   #define SYS_MYNEWCALL 10
   ```

2. Add handler in `kernel/syscall/syscall.c`:
   ```c
   case SYS_MYNEWCALL: {
       // Handle syscall
       return 0;
   }
   ```

3. Call from user code using `ecall` instruction

### Adding a New Shell Command

In `kernel/main.c`, in the `run_shell()` function, add:

```c
else if (/* compare command */) {
    // Handle command
}
```

## Code Style

- **Indentation**: 4 spaces
- **Braces**: K&R style (opening brace on same line)
- **Naming**: snake_case for functions and variables
- **Types**: Use stdint types (uint64_t, etc.)
- **Comments**: C-style /* */ for multi-line, // for single line

## Testing Strategy

### Unit Testing (Manual)
- Memory allocation: `test` command
- UART I/O: Any shell command
- Printf: Various format specifiers
- Trap handling: Can trigger with invalid instructions

### Integration Testing
- Boot sequence: Watch kernel boot messages
- Shell interaction: Try all commands
- Memory management: Allocate/free pages multiple times

### Debugging Tips

1. **Using GDB**:
   ```bash
   # Terminal 1
   make debug
   
   # Terminal 2
   riscv64-unknown-elf-gdb build/kernel.elf
   (gdb) target remote :1234
   (gdb) break kernel_main
   (gdb) continue
   ```

2. **Adding Debug Prints**:
   ```c
   printf("[DEBUG] Variable x = %d\n", x);
   ```

3. **Examining Memory**:
   ```c
   printf("[MEM] Address %p = %x\n", ptr, *(uint64_t*)ptr);
   ```

## Common Issues and Solutions

### Issue: Kernel doesn't boot
- Check linker script addresses match QEMU memory map
- Verify boot.S properly initializes stack
- Ensure all sections are properly aligned

### Issue: Random crashes
- Check for stack overflow (increase stack size in linker script)
- Verify pointer validity before dereferencing
- Look for buffer overruns in string operations

### Issue: No output on UART
- Verify UART base address (0x10000000 for QEMU virt)
- Check that uart_init() is called
- Ensure QEMU is run with `-nographic` flag

## Performance Considerations

### Memory Allocation
- Page allocation: O(1)
- Heap allocation: O(1)
- Consider free list for heap if many small allocations needed

### I/O
- UART is polled (blocking), consider interrupt-driven I/O
- Buffer multiple characters before sending

### Context Switching
- Save/restore minimal context
- Use sscratch for quick context pointer access

## Security Considerations

### Current Status
- No privilege separation (all code runs in S-mode)
- No memory protection (virtual memory not enabled)
- No input validation

### Future Improvements
- Enable SV39 paging for memory protection
- Add user mode for applications
- Validate all user inputs
- Implement stack canaries

## Resources

### RISC-V Documentation
- [RISC-V Specifications](https://riscv.org/technical/specifications/)
- [RISC-V Assembly Programmer's Manual](https://github.com/riscv-non-isa/riscv-asm-manual)
- [RISC-V Privileged ISA](https://github.com/riscv/riscv-isa-manual)

### Similar Projects
- [xv6-riscv](https://github.com/mit-pdos/xv6-riscv) - MIT's teaching OS
- [rCore](https://github.com/rcore-os/rCore) - Rust OS (inspiration for this project)

### QEMU Documentation
- [QEMU RISC-V](https://www.qemu.org/docs/master/system/target-riscv.html)
- [QEMU virt Machine](https://www.qemu.org/docs/master/system/riscv/virt.html)

## Contributing

When contributing:
1. Follow existing code style
2. Add comments for complex logic
3. Test thoroughly
4. Update documentation
5. Keep changes minimal and focused

## License

MIT License - See LICENSE file
