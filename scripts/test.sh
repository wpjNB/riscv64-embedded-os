#!/bin/bash
# Comprehensive test script for RISC-V 64-bit Embedded OS
# This script tests the build, run, and basic functionality of the OS

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test results
PASSED=0
FAILED=0
TOTAL=0

# Function to print test header
print_test() {
    echo
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}TEST: $1${NC}"
    echo -e "${BLUE}========================================${NC}"
    TOTAL=$((TOTAL + 1))
}

# Function to print success
print_success() {
    echo -e "${GREEN}✓ PASSED${NC}: $1"
    PASSED=$((PASSED + 1))
}

# Function to print failure
print_failure() {
    echo -e "${RED}✗ FAILED${NC}: $1"
    FAILED=$((FAILED + 1))
}

# Function to print info
print_info() {
    echo -e "${YELLOW}INFO${NC}: $1"
}

# Change to project directory
cd "$(dirname "$0")/.."

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}RISC-V 64-bit Embedded OS Test Suite${NC}"
echo -e "${BLUE}========================================${NC}"
echo

# Test 1: Check toolchain availability
print_test "Toolchain Availability"
if command -v riscv64-unknown-elf-gcc &> /dev/null; then
    GCC_VERSION=$(riscv64-unknown-elf-gcc --version | head -n 1)
    print_success "RISC-V GCC found: $GCC_VERSION"
else
    print_failure "RISC-V GCC not found"
    echo "Please install: sudo apt-get install gcc-riscv64-unknown-elf"
    exit 1
fi

# Test 2: Check QEMU availability
print_test "QEMU Availability"
if command -v qemu-system-riscv64 &> /dev/null; then
    QEMU_VERSION=$(qemu-system-riscv64 --version | head -n 1)
    print_success "QEMU found: $QEMU_VERSION"
else
    print_failure "QEMU not found"
    echo "Please install: sudo apt-get install qemu-system-misc"
    exit 1
fi

# Test 3: Clean build
print_test "Clean Build Directory"
if make clean > /dev/null 2>&1; then
    print_success "Build directory cleaned"
else
    print_failure "Failed to clean build directory"
fi

# Test 4: Build the kernel
print_test "Kernel Build"
echo "Building kernel..."
if make all > /tmp/build.log 2>&1; then
    print_success "Kernel built successfully"
    if [ -f build/kernel.elf ]; then
        SIZE=$(ls -lh build/kernel.elf | awk '{print $5}')
        print_info "kernel.elf size: $SIZE"
    fi
    if [ -f build/kernel.bin ]; then
        SIZE=$(ls -lh build/kernel.bin | awk '{print $5}')
        print_info "kernel.bin size: $SIZE"
    fi
else
    print_failure "Kernel build failed"
    echo "Build log:"
    cat /tmp/build.log
    exit 1
fi

# Test 5: Check kernel sections
print_test "Kernel Binary Analysis"
if command -v riscv64-unknown-elf-objdump &> /dev/null; then
    echo "Checking kernel sections..."
    riscv64-unknown-elf-objdump -h build/kernel.elf > /tmp/sections.txt
    
    # Check for required sections
    if grep -q ".text" /tmp/sections.txt; then
        print_success "Text section found"
    else
        print_failure "Text section missing"
    fi
    
    if grep -q ".data" /tmp/sections.txt; then
        print_success "Data section found"
    else
        print_failure "Data section missing"
    fi
    
    if grep -q ".bss" /tmp/sections.txt; then
        print_success "BSS section found"
    else
        print_failure "BSS section missing"
    fi
fi

# Test 6: Run kernel in QEMU (with timeout)
print_test "Kernel Execution in QEMU"
echo "Running kernel in QEMU for 5 seconds..."
# Use unbuffer to capture output from QEMU
timeout 5 unbuffer qemu-system-riscv64 -machine virt -nographic -bios none \
    -m 128M -smp 1 -kernel build/kernel.elf > /tmp/qemu_output.txt 2>&1 || true

if [ -f /tmp/qemu_output.txt ]; then
    # Check for expected output
    if grep -q "RISC-V 64-bit Embedded OS" /tmp/qemu_output.txt; then
        print_success "Kernel banner displayed"
    else
        print_failure "Kernel banner not found"
    fi
    
    if grep -q "KERNEL.*Starting RISC-V OS kernel" /tmp/qemu_output.txt; then
        print_success "Kernel started successfully"
    else
        print_failure "Kernel start message not found"
    fi
    
    if grep -q "MM.*Initializing memory manager" /tmp/qemu_output.txt; then
        print_success "Memory manager initialized"
    else
        print_failure "Memory manager initialization not found"
    fi
    
    if grep -q "TRAP.*Initializing trap handling" /tmp/qemu_output.txt; then
        print_success "Trap handling initialized"
    else
        print_failure "Trap handling initialization not found"
    fi
    
    if grep -q "SHELL.*Starting simple shell" /tmp/qemu_output.txt; then
        print_success "Shell started"
    else
        print_failure "Shell start not found"
    fi
    
    if grep -q "TEST.*Memory test completed" /tmp/qemu_output.txt; then
        print_success "Memory test completed"
    else
        print_failure "Memory test not completed"
    fi
    
    echo
    echo "QEMU output:"
    echo "----------------------------------------"
    cat /tmp/qemu_output.txt
    echo "----------------------------------------"
else
    print_failure "No QEMU output captured"
fi

# Test 7: Check for proper memory layout
print_test "Memory Layout Verification"
if riscv64-unknown-elf-nm build/kernel.elf | grep -q "__stack_top"; then
    STACK_TOP=$(riscv64-unknown-elf-nm build/kernel.elf | grep "__stack_top" | awk '{print $1}')
    print_success "Stack top symbol found at 0x$STACK_TOP"
else
    print_failure "Stack top symbol not found"
fi

if riscv64-unknown-elf-nm build/kernel.elf | grep -q "__heap_start"; then
    HEAP_START=$(riscv64-unknown-elf-nm build/kernel.elf | grep "__heap_start" | awk '{print $1}')
    print_success "Heap start symbol found at 0x$HEAP_START"
else
    print_failure "Heap start symbol not found"
fi

if riscv64-unknown-elf-nm build/kernel.elf | grep -q "kernel_main"; then
    KERNEL_MAIN=$(riscv64-unknown-elf-nm build/kernel.elf | grep "kernel_main" | awk '{print $1}')
    print_success "kernel_main found at 0x$KERNEL_MAIN"
else
    print_failure "kernel_main not found"
fi

# Test 8: Source file verification
print_test "Source File Verification"
REQUIRED_FILES=(
    "bootloader/boot.S"
    "kernel/main.c"
    "kernel/mm/mm.c"
    "kernel/trap/trap.c"
    "drivers/uart/uart.c"
    "scripts/kernel.ld"
)

for file in "${REQUIRED_FILES[@]}"; do
    if [ -f "$file" ]; then
        print_success "Found $file"
    else
        print_failure "Missing $file"
    fi
done

# Print summary
echo
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}TEST SUMMARY${NC}"
echo -e "${BLUE}========================================${NC}"
echo -e "Total Tests: $TOTAL"
echo -e "${GREEN}Passed: $PASSED${NC}"
echo -e "${RED}Failed: $FAILED${NC}"
echo

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}✓ All tests passed!${NC}"
    echo
    echo "The RISC-V 64-bit Embedded OS is working correctly!"
    echo "You can:"
    echo "  - Run it with: make run"
    echo "  - Debug it with: make debug"
    echo "  - Clean build with: make clean && make all"
    exit 0
else
    echo -e "${RED}✗ Some tests failed${NC}"
    exit 1
fi
