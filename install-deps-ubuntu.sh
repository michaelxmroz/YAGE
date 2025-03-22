#!/bin/bash

echo "Installing system dependencies for YAGE on Ubuntu/Debian..."

# Update package lists
sudo apt-get update

# Install essential build tools
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    clang \
    lld

# Install X11 development libraries (required for window creation)
sudo apt-get install -y \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev

# Install audio libraries for PortAudio
sudo apt-get install -y \
    libasound2-dev \
    libpulse-dev \
    libjack-jackd2-dev

# Install Vulkan dependencies
sudo apt-get install -y \
    libvulkan-dev \
    vulkan-tools \
    mesa-vulkan-drivers

# Libraries required by vcpkg
sudo apt-get install -y \
    curl \
    zip \
    unzip \
    tar

echo "All dependencies installed successfully!"
echo "You can now run ./build-linux.sh to build the project" 