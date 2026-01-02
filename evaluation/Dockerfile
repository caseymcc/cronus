FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    python3-venv \
    curl \
    git \
    build-essential \
    g++ \
    cmake \
    make \
    nodejs \
    npm \
    && rm -rf /var/lib/apt/lists/*

# Install Python testing tools
RUN pip3 install --no-cache-dir \
    pytest \
    pytest-cov \
    requests

# Install Node.js testing tools globally
RUN npm install -g jest

# Set working directory
WORKDIR /app

# Default command
CMD ["/bin/bash"]
