#!/bin/bash
# Setup script for RISC-V development environment

set -e

# Function to build RISC-V toolchain from source
function build_riscv_toolchain() {
    echo
    echo "Building RISC-V toolchain from source..."
    echo "This will take 30-60 minutes and requires ~10GB of disk space."
    echo
    
    # Create temporary directory
    TEMP_DIR=$(mktemp -d)
    cd "$TEMP_DIR"
    
    # Clone toolchain
    echo "Cloning riscv-gnu-toolchain..."
    git clone --depth 1 https://github.com/riscv/riscv-gnu-toolchain
    cd riscv-gnu-toolchain
    git submodule update --init --depth 1
    
    # Configure and build
    echo "Configuring..."
    ./configure --prefix=/opt/riscv --with-arch=rv64imac --with-abi=lp64
    
    echo "Building... (this will take a while)"
    make -j$(nproc)
    
    echo "Installing (requires sudo)..."
    sudo make install
    
    # Add to PATH
    echo "Adding to PATH..."
    if [ -f "$HOME/.bashrc" ]; then
        echo 'export PATH="/opt/riscv/bin:$PATH"' >> "$HOME/.bashrc"
        echo "Added to ~/.bashrc"
    fi
    if [ -f "$HOME/.zshrc" ]; then
        echo 'export PATH="/opt/riscv/bin:$PATH"' >> "$HOME/.zshrc"
        echo "Added to ~/.zshrc"
    fi
    
    # Cleanup
    cd ~
    rm -rf "$TEMP_DIR"
    
    echo "RISC-V toolchain installed successfully!"
    echo "Please restart your terminal or run: source ~/.bashrc"
}

echo "=========================================="
echo "RISC-V Development Environment Setup"
echo "=========================================="
echo

# Detect OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
else
    echo "Unsupported OS: $OSTYPE"
    exit 1
fi

echo "Detected OS: $OS"
echo

# Check if running on Ubuntu/Debian
if [ "$OS" = "linux" ]; then
    if command -v apt-get &> /dev/null; then
        echo "Installing dependencies via apt-get..."
        echo "This will require sudo privileges."
        echo
        
        # Update package list
        sudo apt-get update
        
        # Install build tools
        echo "Installing build tools..."
        sudo apt-get install -y build-essential git autoconf automake \
            autotools-dev curl python3 libmpc-dev libmpfr-dev libgmp-dev \
            gawk bison flex texinfo gperf libtool patchutils bc \
            zlib1g-dev libexpat-dev
        
        # Install QEMU
        echo "Installing QEMU..."
        sudo apt-get install -y qemu-system-misc
        
        # Check if RISC-V GCC is available in repos
        if apt-cache show gcc-riscv64-unknown-elf &> /dev/null; then
            echo "Installing RISC-V toolchain from repository..."
            sudo apt-get install -y gcc-riscv64-unknown-elf
        else
            echo
            echo "RISC-V toolchain not available in repositories."
            echo "Would you like to build it from source? (y/n)"
            read -r response
            if [[ "$response" =~ ^[Yy]$ ]]; then
                build_riscv_toolchain
            else
                echo "Skipping toolchain installation."
                echo "You'll need to install it manually."
            fi
        fi
    else
        echo "apt-get not found. Please install dependencies manually."
        echo "Required: gcc, make, qemu-system-riscv64, riscv64-unknown-elf-gcc"
        exit 1
    fi
elif [ "$OS" = "macos" ]; then
    if command -v brew &> /dev/null; then
        echo "Installing dependencies via Homebrew..."
        
        # Install QEMU
        brew install qemu
        
        # Install RISC-V toolchain
        brew tap riscv/riscv
        brew install riscv-tools
    else
        echo "Homebrew not found. Please install Homebrew first:"
        echo "  /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
        exit 1
    fi
fi

echo
echo "=========================================="
echo "Verifying installation..."
echo "=========================================="
echo

# Check for QEMU
if command -v qemu-system-riscv64 &> /dev/null; then
    QEMU_VERSION=$(qemu-system-riscv64 --version | head -n 1)
    echo "✓ QEMU found: $QEMU_VERSION"
else
    echo "✗ QEMU not found"
    MISSING=1
fi

# Check for RISC-V toolchain
if command -v riscv64-unknown-elf-gcc &> /dev/null; then
    GCC_VERSION=$(riscv64-unknown-elf-gcc --version | head -n 1)
    echo "✓ RISC-V GCC found: $GCC_VERSION"
elif command -v riscv64-elf-gcc &> /dev/null; then
    GCC_VERSION=$(riscv64-elf-gcc --version | head -n 1)
    echo "✓ RISC-V GCC found: $GCC_VERSION"
    echo "  Note: You may need to modify the Makefile to use riscv64-elf-gcc"
elif command -v riscv64-linux-gnu-gcc &> /dev/null; then
    GCC_VERSION=$(riscv64-linux-gnu-gcc --version | head -n 1)
    echo "✓ RISC-V GCC found: $GCC_VERSION"
    echo "  Note: You may need to modify the Makefile to use riscv64-linux-gnu-gcc"
else
    echo "✗ RISC-V GCC not found"
    MISSING=1
fi

# Check for make
if command -v make &> /dev/null; then
    MAKE_VERSION=$(make --version | head -n 1)
    echo "✓ Make found: $MAKE_VERSION"
else
    echo "✗ Make not found"
    MISSING=1
fi

echo

if [ -n "$MISSING" ]; then
    echo "Some tools are missing. Please install them manually."
    echo
    echo "See docs/BUILD.md for detailed installation instructions."
    exit 1
else
    echo "All tools are installed!"
    echo
    echo "You can now build and run the OS:"
    echo "  make all    # Build the kernel"
    echo "  make run    # Run in QEMU"
    echo "  make debug  # Debug with GDB"
    echo
    echo "See docs/BUILD.md for more information."
fi
