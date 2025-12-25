#!/bin/bash
# Setup script for libdragon on Ubuntu
# Run this on your Ubuntu machine

set -e

echo "=== iQue Homebrew Menu - Development Setup ==="
echo ""

# Check if running on Ubuntu
if ! command -v apt &> /dev/null; then
    echo "Error: This script is designed for Ubuntu/Debian"
    exit 1
fi

# Install dependencies
echo "[1/5] Installing build dependencies..."
sudo apt update
sudo apt install -y build-essential git libpng-dev wget

# Set up libdragon installation directory
export N64_INST=${N64_INST:-$HOME/n64_toolchain}
echo "[2/5] Installing libdragon to: $N64_INST"

# Clone libdragon if not exists
if [ ! -d "$HOME/libdragon" ]; then
    echo "[3/5] Cloning libdragon..."
    cd $HOME
    git clone --branch preview https://github.com/DragonMinded/libdragon.git
else
    echo "[3/5] libdragon already cloned, updating..."
    cd $HOME/libdragon
    git pull
fi

# Build toolchain (this takes a while)
echo "[4/5] Building N64 toolchain (this may take 30-60 minutes)..."
cd $HOME/libdragon
./tools/build_toolchain.sh

# Build libdragon
echo "[5/5] Building libdragon..."
make
make install
make tools
make tools-install

# Add to PATH
echo ""
echo "=== Setup Complete! ==="
echo ""
echo "Add this to your ~/.bashrc:"
echo "  export N64_INST=$N64_INST"
echo "  export PATH=\$N64_INST/bin:\$PATH"
echo ""
echo "Then run: source ~/.bashrc"
echo ""
echo "To build the homebrew menu:"
echo "  cd homebrew/menu"
echo "  make"
echo ""
