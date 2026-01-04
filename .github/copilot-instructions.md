# Copilot Instructions for Cronus Project

This application is a modular AI assistant platform called Cronus, designed to support multiple client interfaces (web, VS Code, CLI) and integrate with various language models through a unified arbiterAI layer. 

main application directory: server/cronus/
dependencies:
- arbiterAI: server/arbiterAI/, unified LLM interface
- loreforge: server/loreforge/, code understanding and search
interactive clients:
- Web client: clients/web/, React-based web interface
- VSCode extension: clients/vscode/, VS Code integration
- CLI: clients/cli/, command-line interface

The web and VSCode clients should use as much shared code as possible to minimize duplication.

## Documentation

**Documentation Index**: A comprehensive index of all project documentation with detailed summaries is available at `docs/README.md`. This file contains:
- Complete table of contents for all documentation files
- Detailed summaries of each document's contents
- Quick links to key documentation areas
- Cross-references between related documents

When you need information about the project, check `docs/README.md` first to find the most relevant documentation file.

Key documentation files:
- `docs/project.md` - Project overview, goals, and core features
- `docs/architecture.md` - System architecture and dual-mode design
- `docs/developer.md` - Build environment and developer guide
- `docs/current_state.md` - Current project status and component maturity
- `docs/README.md` - **Complete documentation index with summaries**

## Build Environment

This project uses a Docker-based development environment. **All build commands must be run inside the Docker container.**

## Testing

The project uses Exercism exercises for agent evaluation and testing.

### Agent Evaluation System

The evaluation system tests agent performance on coding challenges across multiple languages:

- **Location**: `evaluation/`
- **Languages**: C++, Python, JavaScript
- **Source**: Exercism exercise repositories (downloaded separately, not in repo)
- **Runtime**: Docker container with all dependencies (pytest, jest, g++, etc.) - automatic

#### Quick Start

```bash
# 1. Download exercises (one-time setup)
./evaluation/setup.sh

# 2. Configure agent endpoint (edit evaluation/config.json)
# Set "endpoint" to your LLM server (e.g., llama.cpp at http://IP:8000/v1)
# Set "api_type" to "openai" for OpenAI-compatible APIs

# 3. Run evaluation (automatically uses Docker)
./evaluation/run_evaluation.sh --language python

# 4. View results
xdg-open evaluation/results/latest/summary.html
```

#### Key Features
- **Automatic Docker**: Runs in Docker automatically, no manual setup
- **Cronus Agent Integration**: Uses Cronus agent server with specialized coder agents
- **Multi-language**: C++, Python, JavaScript exercises
- **Comprehensive metrics**: Correctness, code quality, performance, completeness
- **Detailed logging**: Captures full agent interactions, prompts, and responses

#### Architecture

```
Evaluation → Cronus Server → Coder Agent → arbiterAI → LLM
             (port 9000)      (specialized)  (unified API)
```

- Automatically builds and starts Cronus server
- Uses specialized code generation agents
- Communicates via REST API + SSE
- Full agent interaction logging

#### Key Files
- `evaluation/setup.sh` - Download Exercism exercises
- `evaluation/run_evaluation.sh` - Run evaluations (auto-Docker)
- `evaluation/config.json` - Configuration (endpoint, languages, limits)
- `evaluation/scripts/cronus_client.py` - Cronus server client with SSE support
- `evaluation/scripts/evaluate_exercise.py` - Core evaluation logic
- `evaluation/QUICKSTART.md` - Detailed usage guide
- `evaluation/README.md` - Full documentation

#### Configuration

```json
{
  "agent": {
    "endpoint": "http://localhost:9000/api",
    "cronus_executable": "build/linux_x64_debug/server/cronus/cronus",
    "cronus_working_dir": "/tmp/cronus_eval"
  }
}
```

See `evaluation/README.md` for comprehensive documentation.




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