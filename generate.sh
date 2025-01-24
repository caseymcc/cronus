#!/bin/bash

# Function to show usage
usage() {
    echo "Usage: $0 [os] [arch] [build-type]"
    echo "  os:         Operating system target (linux, macos, or windows) default: linux"
    echo "  arch:       Architecture target (x64, armv7, or arm64) default: x64"
    echo "  build-type: Build type (debug or release) default: debug"
    echo
    echo "Example: $0 linux x64 debug"
    exit 1
}

# Set default values
OS="${1:-linux}"
ARCH="${2:-x64}"
BUILD_TYPE="${3:-debug}"

# Show usage if help is requested
if [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    usage
fi


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
    x64|armv7|arm64)
        ;;
    *)
        echo "Error: Invalid architecture. Must be x64, armv7, or arm64"
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
    x64)
        ARCH_FLAGS="-DCMAKE_SYSTEM_PROCESSOR=x86_64"
        ;;
    armv7)
        ARCH_FLAGS="-DCMAKE_SYSTEM_PROCESSOR=armv7"
        ;;
    arm64)
        ARCH_FLAGS="-DCMAKE_SYSTEM_PROCESSOR=aarch64"
        ;;
esac

# Validate build type
case $BUILD_TYPE in
    debug|release)
        ;;
    *)
        echo "Error: Invalid build type. Must be debug or release"
        exit 1
        ;;
esac

# Convert build type to CMake format (uppercase)
CMAKE_BUILD_TYPE=$(echo $BUILD_TYPE | tr '[:lower:]' '[:upper:]')

# Configure CMake with appropriate options
cmake .. \
    -G "$GENERATOR" \
    -DCMAKE_SYSTEM_NAME=$OS \
    -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
    $ARCH_FLAGS \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake

echo "CMake configuration complete in $BUILD_DIR"
