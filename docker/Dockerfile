FROM ubuntu:22.04

# Install basic build tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    ninja-build \
    && rm -rf /var/lib/apt/lists/*

# Install vcpkg
WORKDIR /opt
RUN git clone https://github.com/Microsoft/vcpkg.git && \
    ./vcpkg/bootstrap-vcpkg.sh

# Add vcpkg to PATH
ENV PATH="/opt/vcpkg:${PATH}"

# Set working directory
WORKDIR /app

# Copy project files
COPY . .

# Configure CMake to use vcpkg
ENV VCPKG_ROOT=/opt/vcpkg
ENV CMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build command (to be run when container starts)
CMD ["bash"]
