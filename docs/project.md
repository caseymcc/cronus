# Cronus Project Documentation

## Project Overview

**Cronus** is a modular AI assistant platform designed to support multiple client interfaces (web, VS Code, CLI) and integrate with various language models through a unified arbiterAI layer. Cronus operates in two distinct modes to support different development workflows.

### Quick Reference
- **Main Application**: `server/cronus/`
- **Dependencies**: `arbiterAI` (LLM interface), `loreforge` (code understanding)
- **Clients**: Web, VSCode extension, CLI, Debug client
- **Build Environment**: Docker-based (use `./run_local.sh`)
- **Documentation Index**: See [docs/README.md](README.md) for complete documentation index and summaries

---

## Description

This application is a modular AI assistant platform called Cronus, designed to support multiple client interfaces (web, VS Code, CLI) and integrate with various language models through a unified arbiterAI layer. Cronus operates in two distinct modes to support different development workflows.

## Operational Modes

### Single-Agent Mode
When Cronus is started in an **existing repository**, it operates in single-agent mode:
- **Configuration Location**: `.cronus/` directory in the repository root
- **Operation**: Only one agent instance can run and modify the repository at a time
- **Use Case**: Individual developer workflow, single AI assistant for a project
- **State Management**: Agent state, context, and configuration stored in `.cronus/`

### Multi-Agent Mode
When Cronus is started in an **empty directory** or a **directory configured for multi-agent development**:
- **Configuration Location**: `.cronus/` directory in the working directory
- **Operation**: Multiple agents can work in parallel, each with their own repository copy
- **Repository Structure**: Cronus maintains separate repository trees under `.cronus/agents/<agent-id>/`
- **State Management**: Each agent maintains independent state, allowing the server to track multiple agents' status
- **Use Case**: Collaborative AI development, agent competition/comparison, parallel experiments

## Primary Goals
- Generate a C/C++ application that works as a coding AI agent with a web client
- Support both single-agent and multi-agent operational modes
- Maintain state and context for each agent independently in multi-agent mode
- Enable seamless switching between operational modes based on directory context

## Secondary Goals
- A CLI client to the agent server
- A VSCode extension to the agent server (re-using as much as possible from the web client)
- Persistent configuration and state management across server restarts

## Target Audience
Developers interested in AI-assisted development, whether working solo or experimenting with multiple AI agents

## Core Features
- **Dual-Mode Operation**: Single-agent for focused development, multi-agent for experimentation
- **AI Coding Agent Server**: Central server managing agent lifecycle and operations
- **Multiple Client Interfaces**: Web, VSCode, CLI clients for the server
- **Specialized Agents**: Code writer, formatter, debugger, tester, optimizer - each with custom prompts and models
- **State Management System**:
  - Agent state (active, idle, error)
  - Task tracking (todo, in-progress, done)
  - Design decisions and architecture documentation
  - Library dependencies and goals
  - Per-agent state isolation in multi-agent mode
- **Configuration System**:
  - `.cronus/` directory structure for all configuration
  - Mode detection (single vs multi-agent)
  - Per-agent configuration in multi-agent mode
  - Provider configurations (LLM, Embedding, Storage)
  - Custom models with custom prompts
  - Custom tools and code formatting rules per language
- **AI Arbiter Layer**: Manage multiple LLMs (using server/arbiterAI/)
- **Code Understanding**: AI-powered search and analysis (using server/loreforge/)

## Configuration Directory Structure

### Single-Agent Mode (`.cronus/`)
```
.cronus/
├── config.json          # Server and agent configuration
├── state.json           # Current agent state
├── tasks/               # Task tracking
├── context/             # Conversation and context history
└── cache/               # Temporary files and cache
```

### Multi-Agent Mode
```
workspace/
├── .cronus/             # Global configuration
│   ├── config.json      # Global server configuration
│   ├── multi-agent.marker
│   ├── shared/          # Shared resources
│   │   ├── cache/
│   │   └── templates/
│   ├── comparison/      # Agent comparison data
│   └── logs/            # Server logs
│
└── agents/              # Agent workspace directories
    ├── agent-1/
    │   ├── .cronus/     # Agent 1's configuration
    │   │   ├── config.json
    │   │   ├── state.json
    │   │   ├── tasks/
    │   │   ├── context/
    │   │   ├── cache/
    │   │   └── logs/
    │   ├── .git/        # Agent 1's repository
    │   └── src/         # Agent 1's source code
    │
    └── agent-2/
        ├── .cronus/     # Agent 2's configuration
        ├── .git/        # Agent 2's repository
        └── src/         # Agent 2's source code
```

---

## Development Process

### Task Tracking
Tasks are managed in `docs/development/tasks/`:
- **`todo/`**: Planned tasks
- **`current/`**: Active task (one at a time)
- **`completed/`**: Finished tasks

### Workflow
1. Read task schedule for priority
2. Move task from `todo/` to `current/`
3. Implement, test, verify
4. Move to `completed/` with notes

---

## Build Environment

**Docker-based development** - All commands via `./run_local.sh`:

```bash
# Generate build files
./run_local.sh ./generate.sh

# Build project
./run_local.sh ninja -C build/linux_x64_debug

# Run tests
./run_local.sh ninja -C build/linux_x64_debug test

# Access shell
./run_local.sh bash
```

**Key Features**:
- Persistent container (reuse between commands)
- Shared vcpkg cache
- Same UID/GID as host user
- Port mapping for services

---

## Additional Documentation

For a complete index of all documentation with detailed summaries, see [docs/README.md](README.md).

Key documentation files:
- [architecture.md](architecture.md) - Detailed architecture of single-agent and multi-agent modes
- [configuration.md](configuration.md) - Configuration reference for .cronus directory structure
- [developer.md](developer.md) - Project structure, interface, configuration, and build instructions
- [development-workflow.md](development-workflow.md) - Workflow for both operational modes
- [research-decisions.md](research-decisions.md) - Research and design decisions for the agent system

---

*Last Updated: January 4, 2026*
