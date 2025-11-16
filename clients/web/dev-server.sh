#!/bin/bash
# This script starts the React development server with the appropriate proxy settings
# It will run inside the Docker container if it exists, otherwise locally

CONTAINER_NAME="cronus_dev"
DEFAULT_API_URL="http://localhost:8080"

# Check if Docker is available
if command -v docker &> /dev/null; then
  # Check if our container is running
  if docker ps --format '{{.Names}}' | grep -q "^${CONTAINER_NAME}$"; then
    echo "Found running Docker container '${CONTAINER_NAME}'"
    echo "Starting development server inside Docker container..."
    
    # Execute the development server inside the container
    docker exec -it ${CONTAINER_NAME} bash -c "cd /app/web && \
      export REACT_APP_API_URL=$DEFAULT_API_URL && \
      if [ ! -d \"node_modules\" ]; then npm install; fi && \
      npm start -- --host 0.0.0.0"
    
    exit $?
  else
    echo "Docker container '${CONTAINER_NAME}' is not running"
    echo "Starting development server locally..."
  fi
else
  echo "Docker not found, starting development server locally..."
fi

# If we're here, we're running locally

# Default API URL - will be overridden by REACT_APP_API_URL if set in the environment
API_URL="${REACT_APP_API_URL:-$DEFAULT_API_URL}"

# Running inside Docker?
if [ -f /.dockerenv ]; then
  RUNNING_IN_DOCKER=1
  echo "Running inside Docker container"
else
  RUNNING_IN_DOCKER=0
  echo "Running directly on host"
fi

# In Docker and API_URL points to localhost? That won't work, show an error message
if [ $RUNNING_IN_DOCKER -eq 1 ] && [[ "$API_URL" == *"localhost"* ]]; then
  echo "Error: Cannot connect to localhost API from inside Docker."
  echo "Please run the Docker container with the -a flag to enable host API access:"
  echo "./run_local.sh -a"
  echo "Or set REACT_APP_API_URL to point to host.docker.internal instead of localhost"
  exit 1
fi

echo "Connecting to Cronus backend at ${API_URL}"

# Check for node_modules and install dependencies if needed
if [ ! -d "node_modules" ]; then
  echo "Installing dependencies..."
  npm install
fi

# Set the API URL environment variable for the React app
export REACT_APP_API_URL="${API_URL}"

echo "Starting development server..."
echo "Frontend will be available at http://localhost:3000"
echo "API requests will be proxied to ${API_URL}"
echo "Press Ctrl+C to stop the server"

# Start the development server with port exposed to the host if running in Docker
if [ $RUNNING_IN_DOCKER -eq 1 ]; then
  # When in Docker, we need to bind to 0.0.0.0 to allow external access
  npm start -- --host 0.0.0.0
else
  # Regular start on host
  npm start
fi