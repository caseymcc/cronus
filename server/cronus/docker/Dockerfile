FROM ubuntu:22.04

# Install basic build tools and Python 3
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    doxygen \
    zip \
    unzip \
    tar \
    pkg-config \
    ninja-build \
    python3 \
    python3-pip \
    python3-dev \
    python3-venv \
    bison \
    gfortran \
    graphviz \
    libxmu-dev \
    libxi-dev \
    libgl-dev \
    libx11-dev \
    libxcursor-dev \
    libxrandr-dev \
    libxinerama-dev \
    libglu1-mesa-dev \
    && apt-get remove -y nodejs npm \
    && rm -rf /var/lib/apt/lists/*

RUN curl -fsSL https://deb.nodesource.com/setup_18.x | bash - && \
    apt-get install -y nodejs && \
    rm -rf /var/lib/apt/lists/*

# Setup Python environment
ENV PYTHONUNBUFFERED=1
ENV PYTHONDONTWRITEBYTECODE=1

# Install vcpkg
WORKDIR /opt
RUN git clone https://github.com/Microsoft/vcpkg.git && \
    ./vcpkg/bootstrap-vcpkg.sh && \
    chmod -R a+rwX /opt/vcpkg && \
    mkdir -p /opt/vcpkg/buildtrees /opt/vcpkg/packages /opt/vcpkg/downloads && \
    chmod -R 777 /opt/vcpkg/buildtrees /opt/vcpkg/packages /opt/vcpkg/downloads

# Add vcpkg to PATH
ENV PATH="/opt/vcpkg:${PATH}"

# Set working directory
WORKDIR /app

# Configure CMake to use vcpkg
ENV VCPKG_ROOT=/opt/vcpkg
ENV VCPKG_DEFAULT_BINARY_CACHE=/vcpkg_cache

ENV CMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake

# Create entrypoint script to handle user creation
RUN echo '#!/bin/bash\n\
# Create group if it does not exist\n\
if ! getent group $HOST_GID >/dev/null; then\n\
    groupadd -g $HOST_GID $HOST_GROUP\n\
fi\n\
\n\
# Create user if it does not exist\n\
if ! id -u $HOST_UID >/dev/null 2>&1; then\n\
    useradd -u $HOST_UID -g $HOST_GID -m -s /bin/bash $HOST_USER\n\
fi\n\
\n\
# Ensure vcpkg working directories have proper permissions\n\
chmod -R 777 /opt/vcpkg/buildtrees /opt/vcpkg/packages /opt/vcpkg/downloads 2>/dev/null || true\n\
\n\
# Ensure user home directory has proper ownership\n\
chown -R $HOST_UID:$HOST_GID /home/$HOST_USER 2>/dev/null || true\n\
\n\
# Create vcpkg cache directory with proper permissions\n\
mkdir -p /home/$HOST_USER/.cache/vcpkg\n\
chown -R $HOST_UID:$HOST_GID /home/$HOST_USER/.cache 2>/dev/null || true\n\
\n\
# Fix build directory permissions if it exists\n\
if [ -d "/app/build" ]; then\n\
    chown -R $HOST_UID:$HOST_GID /app/build 2>/dev/null || true\n\
fi\n\
\n\
# Execute command as the host user\n\
exec gosu $HOST_UID:$HOST_GID "$@"\n\
' > /usr/local/bin/entrypoint.sh && chmod +x /usr/local/bin/entrypoint.sh

# Install gosu for user switching
RUN apt-get update && apt-get install -y gosu && rm -rf /var/lib/apt/lists/*

ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]

# Build command (to be run when container starts)
CMD ["bash"]
