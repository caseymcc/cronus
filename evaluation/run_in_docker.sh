#!/bin/bash
# Docker wrapper for running evaluations
# This script builds and runs the evaluation container

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

# Container name
CONTAINER_NAME="cronus_eval"
IMAGE_NAME="cronus-evaluation"

# Check if image exists or needs rebuild
if ! docker image inspect ${IMAGE_NAME} >/dev/null 2>&1; then
    log_info "Building evaluation Docker image..."
    docker build -t ${IMAGE_NAME} -f "${SCRIPT_DIR}/Dockerfile" "${SCRIPT_DIR}"
fi

# Remove existing container if running
docker rm -f ${CONTAINER_NAME} 2>/dev/null || true

# Get host network interface IP (for accessing host services)
HOST_IP=$(ip route get 1 | awk '{print $7;exit}')

log_info "Running evaluation in Docker container..."
log_info "Host IP: ${HOST_IP} (for accessing llama.cpp server)"

# Run container with:
# - Project mounted at /app
# - Network access to host
# - Same user permissions
docker run --rm -it \
    --name ${CONTAINER_NAME} \
    --network host \
    --add-host=host.docker.internal:host-gateway \
    -v "${PROJECT_ROOT}:/app" \
    -w /app/evaluation \
    -e HOST_IP="${HOST_IP}" \
    ${IMAGE_NAME} \
    "$@"
