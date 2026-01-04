# Development Roadmap

This document outlines the high-level plan to reach the goals defined in `docs/project.md`.

## Phase 0: Core Architecture - Dual-Mode System
Implement the foundational dual-mode architecture that enables both single-agent and multi-agent workflows.

- **Task 001**: Mode Detection and Initialization
    - Implement directory inspection to detect single-agent vs multi-agent mode
    - Create `.cronus/` directory structure for both modes
    - Initialize appropriate configuration files
- **Task 002**: Configuration Management System
    - Implement configuration file schemas (JSON)
    - Support for global and per-agent configuration
    - Environment variable substitution
    - Configuration validation
- **Task 003**: State Persistence Layer
    - Agent state tracking (active/idle/error)
    - Session management and recovery
    - Task state persistence (todo/in-progress/done)
    - Context and conversation history storage

## Phase 1: Single-Agent Mode Implementation
Build the foundation for individual developer workflows.

- **Task 004**: Single-Agent Core
    - Agent lifecycle management (start/stop/restart)
    - File modification and tracking
    - Lock mechanism to prevent concurrent instances
    - Integration with loreforge for code understanding
- **Task 005**: Task Management System
    - Task tracking (todo/in-progress/done)
    - Automatic task extraction from conversations
    - Task persistence across sessions

## Phase 2: Multi-Agent Mode Implementation
Enable parallel agent experimentation and comparison.

- **Task 006**: Multi-Agent Infrastructure
    - Agent workspace isolation (`agents/<agent-id>/`)
    - Repository cloning and initialization
    - Per-agent `.cronus/` configuration
    - Parallel agent execution
- **Task 007**: Agent Comparison System
    - Metrics collection (correctness, performance, quality)
    - Diff generation between agent outputs
    - Comparison report generation (HTML/JSON)
    - Agent performance tracking

## Phase 3: Client Expansion
Expand the reach of the Cronus server by implementing additional clients.

- **Task 008**: Web Client Enhancement
    - Mode-aware UI (single-agent vs multi-agent)
    - Agent status dashboard (multi-agent mode)
    - Real-time updates via SSE
    - Comparison visualization
- **Task 009**: CLI Client
    - Mode detection and selection
    - Agent-specific commands (multi-agent mode)
    - Interactive and non-interactive modes
    - Configuration management commands
- **Task 010**: VS Code Extension
    - Workspace integration
    - Mode detection
    - In-editor agent interaction
    - Re-use Web Client components

## Phase 4: Agent Capabilities & Configuration
Deepen the intelligence and flexibility of the system.

- **Task 011**: Advanced Configuration System
    - Custom prompts per agent type
    - Tool configuration and permissions
    - Model selection and parameters
    - Code formatting rules per language
- **Task 012**: Specialized Agents
    - Coder agent (primary implementation)
    - Formatter agent
    - Debugger agent
    - Tester agent
    - Optimizer agent
    - Custom agent types

## Phase 5: Advanced Features
Realize the goal of a self-managing development process.

- **Task 013**: Repository Synchronization (Multi-Agent)
    - Git integration for agent repositories
    - Branch-based change tracking
    - Pull/push to source repository
    - Merge conflict detection
- **Task 014**: Agent Collaboration (Future)
    - Inter-agent communication
    - Coordinated workflows (one writes, another reviews)
    - Shared context and knowledge
- **Task 015**: Cloud Integration (Future)
    - Multi-agent workspace sync to cloud
    - Distributed agent execution
    - Remote model access

## Current Focus (January 2026)

**Immediate Priority**: Phase 0 - Core Architecture
- Mode detection implementation
- Configuration system design and implementation
- State persistence layer

**Next Up**: Phase 1 - Single-Agent Mode
- Core agent functionality
- File operations and tracking
- Basic task management
