#!/bin/bash

CONTAINER_NAME="cronus_dev"

# Function to show usage
usage() {
    echo "Usage: $0 [-r] [-s]"
    echo "  -r: Rebuild Docker image"
    echo "  -s: Stop running container before starting"
    echo "  -v: Path to vcpkg installation on host system"
    exit 1
}

# Parse command line options
REBUILD=0
STOP=0

while getopts "rs" opt; do
    case $opt in
        r) REBUILD=1 ;;
        s) STOP=1 ;;
        ?) usage ;;
    esac
done

# Stop container if requested
if [ $STOP -eq 1 ]; then
    echo "Stopping existing container..."
    docker stop $CONTAINER_NAME 2>/dev/null
fi

# Check if image exists or rebuild is requested
if [ $REBUILD -eq 1 ] || ! docker image inspect cronus >/dev/null 2>&1; then
    echo "Building Docker image..."
    docker build -t cronus -f docker/Dockerfile .
fi

# Start container
echo "Starting development container..."
docker run -it --rm \
    --name $CONTAINER_NAME \
    -v $(pwd):/app \
    -v "$VCPKG_CACHE_DIR":/vcpkg_cache \
    cronus
