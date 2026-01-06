#!/bin/bash

CONTAINER_NAME="cronus_dev"
DEFAULT_CACHE_DIR="$HOME/.cache/vcpkg"
VCPKG_CACHE_DIR="${VCPKG_CACHE_DIR:-$DEFAULT_CACHE_DIR}"
HOST_API_PORT=9000  # Default port for the host API
NOTIFICATION_PORT=8999  # Default port for notifications

# Function to show usage
usage() {
    echo "Usage: $0 [-r] [-s] [-v PATH] [-p PORT] [-a] [-w] [command...]"
    echo "  -r: Rebuild Docker image (stops and removes container and image)"
    echo "  -s: Stop and remove running container"
    echo "  -v: Path to vcpkg installation on host system"
    echo "  -p: Port where Cronus API is running on host (default: 9000)"
    echo "  -n: Notification port (default: 8999)"
    echo "  -a: Start the standalone Electron app (clients/app)"
    echo "  -w: Start the web client in development mode (port 3000)"
    echo ""
    echo "Examples:"
    echo "  $0                       # Start interactive shell in container"
    echo "  $0 -r                    # Rebuild Docker image from scratch"
    echo "  $0 -s                    # Stop and remove container"
    echo "  $0 -a                    # Build and start standalone app"
    echo "  $0 -w                    # Start web dev server on port 3000"
    echo "  $0 bash -c 'ls -la'      # Run command in container"
    exit 1
}

# Parse command line options
REBUILD=0
STOP=0
STANDALONE_APP=0
WEB_DEV=0

while getopts "rsv:p:n:aw" opt; do
    case $opt in
        r) REBUILD=1 ;;
        s) STOP=1 ;;
        v) VCPKG_CACHE_DIR="$OPTARG" ;;
        p) HOST_API_PORT="$OPTARG" ;;
        n) NOTIFICATION_PORT="$OPTARG" ;;
        a) STANDALONE_APP=1 ;;
        w) WEB_DEV=1 ;;
        ?) usage ;;
    esac
done

# Shift past the parsed options to get remaining arguments
shift $((OPTIND-1))

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

# Set up network options for container
NETWORK_OPTS="--add-host=host.docker.internal:host-gateway"
NETWORK_OPTS="$NETWORK_OPTS -e REACT_APP_API_URL=http://host.docker.internal:$HOST_API_PORT"
NETWORK_OPTS="$NETWORK_OPTS -e NOTIFICATION_PORT=$NOTIFICATION_PORT"

# Step 1: Handle STOP or REBUILD flags
if [ $STOP -eq 1 ] || [ $REBUILD -eq 1 ]; then
    # Stop and remove container if it exists
    if docker ps -a --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
        echo "Stopping and removing container..."
        docker rm -f $CONTAINER_NAME 2>/dev/null
    fi
fi

# Step 2: Handle REBUILD flag - remove image and rebuild
if [ $REBUILD -eq 1 ]; then
    # Remove image if it exists
    if docker image inspect cronus >/dev/null 2>&1; then
        echo "Removing existing Docker image for rebuild..."
        docker rmi cronus 2>/dev/null
    fi
    
    echo "Rebuilding Docker image..."
    docker build -t cronus -f server/cronus/docker/Dockerfile .
    if [ $? -ne 0 ]; then
        echo "Error: Failed to build Docker image"
        exit 1
    fi
fi

# Step 3: Ensure Docker image exists (build if not)
if ! docker image inspect cronus >/dev/null 2>&1; then
    echo "Docker image not found. Building Docker image..."
    docker build -t cronus -f server/cronus/docker/Dockerfile .
    if [ $? -ne 0 ]; then
        echo "Error: Failed to build Docker image"
        exit 1
    fi
fi

# Function to start container in background
start_container()
{
    echo "Starting development container..."
    echo "Using host user: $HOST_USER (UID: $HOST_UID, GID: $HOST_GID)"
    
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
        -p $HOST_API_PORT:9000 \
        -p $NOTIFICATION_PORT:8999 \
        $NETWORK_OPTS \
        cronus tail -f /dev/null
    
    # Wait a moment for container to start
    sleep 2
}

# Step 4: Ensure container is running
if ! docker ps --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
    start_container
else
    echo "Using existing container: $CONTAINER_NAME"
fi

# Step 5: Handle web development mode
if [ $WEB_DEV -eq 1 ]; then
    echo "Starting web client in development mode..."
    echo "Web dev server will be available at http://localhost:3000"
    echo "Backend API should be running on http://localhost:$HOST_API_PORT"
    echo ""
    
    # Install dependencies if needed and start dev server
    echo "Installing dependencies and starting React dev server..."
    docker exec -it $CONTAINER_NAME bash -c "
        cd /app/clients/web && \
        if [ ! -d node_modules ]; then npm install; fi && \
        npm start
    "
    
    exit $?
fi

# Step 6: Handle standalone app launch
if [ $STANDALONE_APP -eq 1 ]; then
    echo "Building and starting standalone Electron app..."
    
    # First, ensure dependencies are built in Docker
    echo "Building shared libraries in Docker..."
    docker exec -it $CONTAINER_NAME bash -c "cd /app && ./clients/build-app.sh"
    
    if [ $? -ne 0 ]; then
        echo "Error: Failed to build app dependencies"
        exit 1
    fi
    
    # Start the app inside Docker with X11 forwarding
    echo "Starting Cronus standalone app in Docker with X11 display..."
    
    # Allow X11 connections from Docker
    xhost +local:docker 2>/dev/null || echo "Warning: xhost not available, X11 forwarding may not work"
    
    # Run Electron in Docker with X11 display
    docker exec -it \
        -e DISPLAY=$DISPLAY \
        -e CRONUS_SERVER_URL=http://localhost:9000 \
        -e NOTIFICATION_PORT=$NOTIFICATION_PORT \
        $CONTAINER_NAME \
        bash -c "cd /app/clients/app && npm start"
    
    exit $?
fi

# Step 7: Execute command or open shell
if [ $# -eq 0 ]; then
    # No command specified, open interactive shell
    docker exec -it $CONTAINER_NAME bash
else
    # Execute the specified command
    docker exec -it $CONTAINER_NAME "$@"
fi
