#!/bin/bash

# Function to show usage
usage() {
    echo "Usage: $0 --os <linux|macos|windows> --arch <x86_64|armv7|arm64>"
    echo
    echo "Options:"
    echo "  --os    Operating system target (linux, macos, or windows)"
    echo "  --arch  Architecture target (x86_64, armv7, or arm64)"
    exit 1
}

# Set default values
OS="linux"
ARCH="x86_64"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --os)
            OS="$2"
            shift 2
            ;;
        --arch)
            ARCH="$2"
            shift 2
            ;;
        *)
            usage
            ;;
    esac
done


# Validate OS
case $OS in
    linux|macos|windows)
        ;;
    *)
        echo "Error: Invalid OS. Must be linux, macos, or windows"
        exit 1
        ;;
esac

# Validate architecture
case $ARCH in
    x86_64|armv7|arm64)
        ;;
    *)
        echo "Error: Invalid architecture. Must be x86_64, armv7, or arm64"
        exit 1
        ;;
esac

# Create build directory name
BUILD_DIR="build_${OS}_${ARCH}"

# Create build directory if it doesn't exist
mkdir -p $BUILD_DIR

# Set CMake generator
if [ "$OS" = "windows" ]; then
    GENERATOR="Visual Studio 17 2022"
else
    GENERATOR="Unix Makefiles"
fi

# Configure CMake based on OS and architecture
echo "Configuring CMake for $OS on $ARCH..."
cd $BUILD_DIR

# Set architecture-specific flags
case $ARCH in
    x86_64)
        ARCH_FLAGS="-DCMAKE_SYSTEM_PROCESSOR=x86_64"
        ;;
    armv7)
        ARCH_FLAGS="-DCMAKE_SYSTEM_PROCESSOR=armv7"
        ;;
    arm64)
        ARCH_FLAGS="-DCMAKE_SYSTEM_PROCESSOR=aarch64"
        ;;
esac

# Configure CMake with appropriate options
cmake .. \
    -G "$GENERATOR" \
    -DCMAKE_SYSTEM_NAME=$OS \
    $ARCH_FLAGS \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake

echo "CMake configuration complete in $BUILD_DIR"
