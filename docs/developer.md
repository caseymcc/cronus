main application directory: server/cronus/
dependencies:
- arbiterAI: server/arbiterAI/, unified LLM interface
- loreforge: server/loreforge/, code understanding and search
interactive clients:
- Web client: clients/web/, React-based web interface
- VSCode extension: clients/vscode/, VS Code integration
- CLI: clients/cli/, command-line interface

The web and VSCode clients should use as much shared code as possible to minimize duplication.

## Build Environment

This project uses a Docker-based development environment. **All build commands must be run inside the Docker container.**

### Running Commands in Docker

Use `./run_local.sh` to execute commands inside the Docker container. The script automatically reuses an existing container if one is running, so you don't need to restart Docker for each command:

```bash
# Run build script
./run_local.sh ./generate.sh

# Run ninja build
./run_local.sh ninja -C build/linux_x64_debug

# Run CMake
./run_local.sh cmake -B build/linux_x64_debug

# Access shell in container
./run_local.sh bash

# Run any other command
./run_local.sh <command> <args>
```

The container will persist between commands, making subsequent builds much faster. The container only needs to be restarted when:
- Docker image needs to be rebuilt (use `-r` flag)
- You explicitly stop it (use `-s` flag)
- The container crashes or is manually stopped

### Common Build Commands

- **Generate build files**: `./run_local.sh ./generate.sh`
- **Build project**: `./run_local.sh ninja -C build/linux_x64_debug`
- **Rebuild Docker image**: `./run_local.sh -r`
- **Stop existing container**: `./run_local.sh -s`
- **Run tests**: `./run_local.sh ninja -C build/linux_x64_debug test`
- **Stop container manually**: `docker stop cronus_dev` or `docker rm -f cronus_dev`

### Important Notes

- **DO NOT** run `generate.sh`, `ninja`, `cmake`, or other build tools directly on the host system
- **ALWAYS** prefix build commands with `./run_local.sh`
- The container automatically persists between commands for faster execution
- The Docker container mounts the project directory at `/app`, so all file changes persist
- The container uses the same UID/GID as the host user to avoid permission issues
- vcpkg cache is shared between host and container for faster builds
- Only use `-r` flag when you need to rebuild the Docker image itself (e.g., after Dockerfile changes)
- Only use `-s` flag when you need to force restart the container

### Available Options

- `-r`: Rebuild Docker image before running
- `-s`: Stop and remove existing container before starting
- `-v <path>`: Specify custom vcpkg cache directory
- `-p <port>`: Specify host API port (default: 9000)

### Web Development Server

For web client development, the container exposes port 3000 and can access the host's API through `host.docker.internal`.

## Coding Style Guidelines

- When using a programing language for this project read the coding style guideline for lanuages that languages:
  - C/C++ - read docs/coding-style/cpp.md
  - javascrip/typescript - read docs/coding-style/javascript.md