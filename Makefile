# RISC-V 64-bit Embedded OS Makefile

# Toolchain
CROSS_COMPILE ?= riscv64-unknown-elf-
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump

# Directories
BUILD_DIR := build
KERNEL_DIR := kernel
BOOT_DIR := bootloader
USER_DIR := user
DRIVER_DIR := drivers

# Compiler flags
CFLAGS := -march=rv64imac -mabi=lp64 -mcmodel=medany
CFLAGS += -Wall -Wextra -O2 -g
CFLAGS += -fno-builtin -nostdlib -nostartfiles
CFLAGS += -fno-common -ffunction-sections -fdata-sections
CFLAGS += -I$(KERNEL_DIR) -I$(DRIVER_DIR) -Iinclude

# Linker flags
LDFLAGS := -nostdlib -static
LDFLAGS += --gc-sections

# Qemu settings
QEMU := qemu-system-riscv64
QEMU_FLAGS := -machine virt -nographic -bios none
QEMU_FLAGS += -m 128M -smp 1
QEMU_FLAGS += -kernel $(BUILD_DIR)/kernel.elf

# Kernel sources
KERNEL_SRCS := $(wildcard $(KERNEL_DIR)/*.c)
KERNEL_SRCS += $(wildcard $(KERNEL_DIR)/mm/*.c)
KERNEL_SRCS += $(wildcard $(KERNEL_DIR)/process/*.c)
KERNEL_SRCS += $(wildcard $(KERNEL_DIR)/syscall/*.c)
KERNEL_SRCS += $(wildcard $(KERNEL_DIR)/trap/*.c)
KERNEL_SRCS += $(wildcard $(KERNEL_DIR)/fs/*.c)

# Driver sources
DRIVER_SRCS := $(wildcard $(DRIVER_DIR)/uart/*.c)
DRIVER_SRCS += $(wildcard $(DRIVER_DIR)/rtc/*.c)
DRIVER_SRCS += $(wildcard $(DRIVER_DIR)/plic/*.c)
DRIVER_SRCS += $(wildcard $(DRIVER_DIR)/testdev/*.c)

# Boot sources
BOOT_SRCS := $(wildcard $(BOOT_DIR)/*.S)
BOOT_SRCS += $(wildcard $(BOOT_DIR)/*.c)

# All kernel objects
KERNEL_OBJS := $(KERNEL_SRCS:%.c=$(BUILD_DIR)/%.o)
DRIVER_OBJS := $(DRIVER_SRCS:%.c=$(BUILD_DIR)/%.o)
BOOT_OBJS := $(BOOT_SRCS:$(BOOT_DIR)/%.S=$(BUILD_DIR)/$(BOOT_DIR)/%.o)
BOOT_OBJS += $(filter %.o,$(BOOT_SRCS:$(BOOT_DIR)/%.c=$(BUILD_DIR)/$(BOOT_DIR)/%.o))

ALL_OBJS := $(BOOT_OBJS) $(KERNEL_OBJS) $(DRIVER_OBJS)

# Default target
.PHONY: all
all: $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.bin

# Create build directories
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/$(KERNEL_DIR)/{mm,process,syscall,trap,fs}
	@mkdir -p $(BUILD_DIR)/$(DRIVER_DIR)/{uart,rtc,plic,testdev}
	@mkdir -p $(BUILD_DIR)/$(BOOT_DIR)
	@mkdir -p $(BUILD_DIR)/$(USER_DIR)

# Compile C files
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@echo "CC $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

# Compile assembly files
$(BUILD_DIR)/%.o: %.S | $(BUILD_DIR)
	@echo "AS $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

# Link kernel
$(BUILD_DIR)/kernel.elf: $(ALL_OBJS) scripts/kernel.ld | $(BUILD_DIR)
	@echo "LD $@"
	@$(LD) $(LDFLAGS) -T scripts/kernel.ld $(ALL_OBJS) -o $@
	@$(OBJDUMP) -d $@ > $(BUILD_DIR)/kernel.asm

# Create binary
$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.elf
	@echo "OBJCOPY $@"
	@$(OBJCOPY) -O binary $< $@

# Run in QEMU
.PHONY: run
run: $(BUILD_DIR)/kernel.elf
	@echo "Running in QEMU..."
	@$(QEMU) $(QEMU_FLAGS)

# Debug in QEMU
.PHONY: debug
debug: $(BUILD_DIR)/kernel.elf
	@echo "Starting QEMU in debug mode..."
	@$(QEMU) $(QEMU_FLAGS) -s -S

# Clean build artifacts
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)

# Print variables (for debugging)
.PHONY: print-%
print-%:
	@echo $* = $($*)
