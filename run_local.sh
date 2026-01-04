#!/bin/bash

CONTAINER_NAME="cronus_dev"
DEFAULT_CACHE_DIR="$HOME/.cache/vcpkg"
VCPKG_CACHE_DIR="${VCPKG_CACHE_DIR:-$DEFAULT_CACHE_DIR}"
HOST_API_PORT=9000  # Default port for the host API

# Function to show usage
usage() {
    echo "Usage: $0 [-r] [-s] [-a] [-p PORT] [-d] [command...]"
    echo "  -r: Rebuild Docker image"
    echo "  -s: Stop running container before starting"
    echo "  -v: Path to vcpkg installation on host system"
    echo "  -a: Enable access to host API (for development server)"
    echo "  -p: Port where Cronus API is running on host (default: 9000)"
    echo "  -d: Start the debug client (runs on host, not in Docker)"
    echo ""
    echo "Examples:"
    echo "  $0                       # Start interactive shell in container"
    echo "  $0 -d                    # Build and start debug client on host"
    echo "  $0 bash -c 'ls -la'      # Run command in container"
    exit 1
}

# Parse command line options
REBUILD=0
STOP=0
DEBUG_CLIENT=0

while getopts "rsv:ap:d" opt; do
    case $opt in
        r) REBUILD=1 ;;
        s) STOP=1 ;;
        v) VCPKG_CACHE_DIR="$OPTARG" ;;
        p) HOST_API_PORT="$OPTARG" ;;
        d) DEBUG_CLIENT=1 ;;
        ?) usage ;;
    esac
done

# Shift past the parsed options to get remaining arguments
shift $((OPTIND-1))

# Handle debug client launch
if [ $DEBUG_CLIENT -eq 1 ]; then
    echo "Building and starting debug client..."
    
    # First, ensure dependencies are built in Docker
    echo "Building shared libraries in Docker..."
    $0 ./clients/build-debug-client.sh
    
    if [ $? -ne 0 ]; then
        echo "Error: Failed to build debug client dependencies"
        exit 1
    fi
    
    # Start the debug client inside Docker with X11 forwarding
    echo "Starting debug client in Docker with X11 display..."
    
    # Allow X11 connections from Docker
    xhost +local:docker 2>/dev/null || echo "Warning: xhost not available, X11 forwarding may not work"
    
    # Run Electron in Docker with X11 display
    docker exec -it \
        -e DISPLAY=$DISPLAY \
        $CONTAINER_NAME \
        bash -c "cd /app/clients/debug && npm start"
    
    exit $?
fi

# Remove existing container if requested
if [ $STOP -eq 1 ]; then
    echo "Removing existing container..."
    docker rm -f $CONTAINER_NAME 2>/dev/null
fi

# Check if image exists or rebuild is requested
if [ $REBUILD -eq 1 ] || ! docker image inspect cronus >/dev/null 2>&1; then
    echo "Building Docker image..."
    docker build -t cronus -f server/cronus/docker/Dockerfile .
fi

# Set up network options for container
NETWORK_OPTS="--add-host=host.docker.internal:host-gateway"
NETWORK_OPTS="$NETWORK_OPTS -e REACT_APP_API_URL=http://host.docker.internal:$HOST_API_PORT"

# Get host user ID and group ID
HOST_UID=$(id -u)
HOST_GID=$(id -g)
HOST_USER=$(id -un)
HOST_GROUP=$(id -gn)

# Set up Git configuration mounts
GIT_MOUNTS=""
if [ -f "$HOME/.gitconfig" ]; then
    GIT_MOUNTS="$GIT_MOUNTS -v $HOME/.gitconfig:/home/$HOST_USER/.gitconfig:ro"
fi
if [ -d "$HOME/.ssh" ]; then
    GIT_MOUNTS="$GIT_MOUNTS -v $HOME/.ssh:/home/$HOST_USER/.ssh:ro"
fi
if [ -f "$HOME/.gitconfig.user" ]; then
    GIT_MOUNTS="$GIT_MOUNTS -v $HOME/.gitconfig.user:/home/$HOST_USER/.gitconfig.user:ro"
fi
if [ -f "$HOME/.git-credentials" ]; then
    GIT_MOUNTS="$GIT_MOUNTS -v $HOME/.git-credentials:/home/$HOST_USER/.git-credentials:ro"
fi
if [ -n "$SSH_AUTH_SOCK" ]; then
    GIT_MOUNTS="$GIT_MOUNTS -v $SSH_AUTH_SOCK:/ssh-agent -e SSH_AUTH_SOCK=/ssh-agent"
fi

# Set up X11 forwarding for GUI applications
X11_OPTS=""
if [ -n "$DISPLAY" ]; then
    X11_OPTS="-e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix:rw"
fi

# Check if container is already running
if docker ps --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
    echo "Using existing container: $CONTAINER_NAME"
    # If no command specified, open a shell
    if [ $# -eq 0 ]; then
        docker exec -it $CONTAINER_NAME bash
    else
        docker exec -it $CONTAINER_NAME "$@"
    fi
else
    # Start container
    echo "Starting development container..."
    echo "Using host user: $HOST_USER (UID: $HOST_UID, GID: $HOST_GID)"
    
    # If no command specified, run interactive shell
    if [ $# -eq 0 ]; then
        docker run -it --rm \
            --name $CONTAINER_NAME \
            -e HOST_UID=$HOST_UID \
            -e HOST_GID=$HOST_GID \
            -e HOST_USER=$HOST_USER \
            -e HOST_GROUP=$HOST_GROUP \
            -v $(pwd):/app \
            -v "$VCPKG_CACHE_DIR":/vcpkg_cache \
            $GIT_MOUNTS \
            $X11_OPTS \
            -p 3000:3000 \
            $NETWORK_OPTS \
            cronus
    else
        # Run command and keep container running in background
        docker run -d \
            --name $CONTAINER_NAME \
            -e HOST_UID=$HOST_UID \
            -e HOST_GID=$HOST_GID \
            -e HOST_USER=$HOST_USER \
            -e HOST_GROUP=$HOST_GROUP \
            -v $(pwd):/app \
            -v "$VCPKG_CACHE_DIR":/vcpkg_cache \
            $GIT_MOUNTS \
            $X11_OPTS \
            -p 3000:3000 \
            $NETWORK_OPTS \
            cronus tail -f /dev/null
        
        # Wait a moment for container to start
        sleep 1
        
        # Execute the command
        docker exec -it $CONTAINER_NAME "$@"
    fi
fi
