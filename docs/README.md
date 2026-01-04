# Cronus Documentation Index

This document provides a comprehensive index of all documentation files in the Cronus project with summaries of their contents.

---

## Table of Contents

### Core Documentation
1. [Architecture](architecture.md) - System architecture and dual-mode design
2. [Current State](current_state.md) - Project status and component maturity
3. [Developer Guide](developer.md) - Build environment and coding standards
4. [Development Workflow](development-workflow.md) - Development and testing procedures
5. [Development Process](development.md) - Task tracking and workflow

### API & Integration
6. [API Documentation](api-documentation.md) - gRPC and REST API reference
7. [LLM Integration](llm-integration.md) - ArbiterAI library and model management
8. [WebSocket JSON-RPC API](websocket-jsonrpc-api.md) - Real-time communication protocol

### Client Development
9. [Client Development](client-development.md) - Shared library architecture
10. [Quick Start - Clients](QUICKSTART-CLIENTS.md) - 5-minute client setup guide
11. [Debug Client Complete](DEBUG-CLIENT-COMPLETE.md) - Standalone Electron debug application

### Configuration & Setup
12. [Configuration](configuration.md) - Complete configuration reference
13. [Docker Build Fixes](DOCKER-BUILD-FIXES.md) - Build system troubleshooting

### Planning & Research
14. [Roadmap](development/ROADMAP.md) - Development phases and milestones
15. [Research Decisions](research-decisions.md) - Technology choices and rationale
16. [Features](ideas/features.txt) - Implemented and planned features

### Implementation Summaries
17. [Implementation Summary](IMPLEMENTATION-SUMMARY.md) - Client libraries implementation
18. [Standalone Debug Client](STANDALONE-DEBUG-CLIENT.md) - Electron app implementation

---

## Document Summaries

### Architecture
**File**: `docs/architecture.md` (760 lines)

Comprehensive guide to Cronus's dual-mode architecture:
- **Single-Agent Mode**: One AI assistant per repository with `.cronus/` configuration
- **Multi-Agent Mode**: Multiple agents with isolated repository trees for parallel experimentation
- **Mode Detection**: Automatic detection based on directory state (existing repo vs empty/multi-agent)
- **Configuration Management**: Per-mode directory structures and state persistence
- **Component Architecture**: Server, clients, arbiterAI, and loreforge integration
- **State Management**: Agent lifecycle, task tracking, context history

**Key Topics**:
- Operational mode comparison
- Directory structure for each mode
- Agent state schemas (JSON)
- Configuration file formats
- Mode transition scenarios

---

### Current State
**File**: `docs/current_state.md` (168 lines)

Current project status and component maturity levels:
- **Cronus Server**: Alpha development, CLI client functional
- **ArbiterAI**: Mature (Phase 4) - Production-ready LLM abstraction
- **Loreforge**: Beta (Phase 3) - RAG pipeline complete, gRPC in progress
- **Mode System**: Design complete, implementation pending
- **Configuration**: Schemas defined, persistence layer in progress

**Component Status**:
- Single-agent mode: Ready for implementation
- Multi-agent mode: Architecture defined
- CLI client: Functional with SSE support
- Web/VSCode clients: In development

**Next Milestones**:
- Configuration system implementation (2-3 weeks)
- Mode detection and initialization
- State persistence layer
- Multi-agent repository isolation

---

### Developer Guide
**File**: `docs/developer.md`

Essential developer information:
- **Docker Environment**: All builds must run in Docker via `./run_local.sh`
- **Build Commands**: `./generate.sh`, `ninja -C build/linux_x64_debug`
- **Container Management**: Persistent container, reuse between commands
- **Coding Standards**: Language-specific style guides (C++, JavaScript/TypeScript)
- **Dependencies**: vcpkg for C++, npm for JavaScript/TypeScript

**Key Practices**:
- Never run build tools directly on host
- Always use `./run_local.sh` wrapper
- Container persists for faster builds
- Shared vcpkg cache between host and container

---

### Development Workflow
**File**: `docs/development-workflow.md` (936 lines)

Complete development and testing procedures:
- **Development Environment**: Docker, CMake, vcpkg setup
- **Single-Agent Workflow**: Initialize, develop, commit in existing repos
- **Multi-Agent Workflow**: Create workspace, spawn agents, compare results
- **Testing**: Unit tests, integration tests, agent evaluation
- **Git Integration**: Branch management, change tracking

**Workflows**:
- Feature development in single-agent mode
- Parallel experimentation in multi-agent mode
- Code review and comparison
- Performance benchmarking

---

### Development Process
**File**: `docs/development.md`

Task tracking and development workflow:
- **Task Directory Structure**: `tasks/todo/`, `tasks/current/`, `tasks/completed/`
- **Workflow**: Select task → Work → Complete → Document
- **Task File Format**: Markdown with title, description, acceptance criteria, progress

---

### API Documentation
**File**: `docs/api-documentation.md` (869 lines)

Comprehensive API reference for both gRPC and REST:

**gRPC API (LoreForge)**:
- `HealthCheck`: Server status verification
- `ParseFile`: Source code parsing and validation
- `QueryCodebase`: Natural language code queries with RAG
- `IndexRepository`: Repository indexing for semantic search

**REST API (Cronus)**:
- Agent management endpoints
- File operations (read, write, list)
- Task management (create, update, status)
- Configuration endpoints

**Features**:
- Protocol buffer schemas
- Request/response examples
- Streaming response patterns
- Error handling

---

### LLM Integration
**File**: `docs/llm-integration.md` (782 lines)

ArbiterAI library documentation and LLM provider integration:

**ModelManager**:
- Centralized model configuration and discovery
- Provider-model mappings
- Version compatibility handling
- Priority ranking and selection
- Remote configuration repository

**Supported Providers**:
- OpenAI (GPT-3.5, GPT-4)
- Anthropic (Claude)
- Google (Gemini)
- Local LLMs (llama.cpp)
- Custom providers

**Configuration**:
- Model definitions (JSON)
- Pricing information
- Context windows and token limits
- API endpoints and authentication
- Few-shot example handling

---

### WebSocket JSON-RPC API
**File**: `docs/websocket-jsonrpc-api.md` (291 lines)

Real-time communication protocol specification:
- **Transport**: WebSocket with JSON-RPC 2.0 messages
- **Endpoint**: `ws://localhost:<port>/ws`
- **Bidirectional**: Request/response and server notifications

**Methods**:
- `input`: Send user messages to agent
- `getSourceMap`: Retrieve file tree structure
- `getFile`: Get file content
- `getDirectory`: List directory contents
- `createFile`, `updateFile`, `deleteFile`: File operations

**Event Notifications**:
- Agent responses (streaming)
- File system changes
- Task updates
- Connection status

---

### Client Development
**File**: `docs/client-development.md` (526 lines)

Shared library architecture for Cronus clients:

**Architecture Philosophy**:
- Core logic in `@cronus/shared` (API, models, utilities)
- UI logic in `@cronus/shared-ui` (React components, hooks)
- Platform-specific code in individual clients
- 70-80% code reuse across platforms

**Monorepo Structure**:
- NPM workspaces for dependency management
- Coordinated build scripts
- Watch mode for development
- TypeScript across entire stack

**Shared Components**:
- `CronusClient`: Unified API client
- `SSEManager`: Server-Sent Events handling
- React hooks: `useCronusConnection`, `useFileTree`, `useMessages`
- UI components: `ConnectionStatus`, `LogViewer`

---

### Quick Start - Clients
**File**: `docs/QUICKSTART-CLIENTS.md` (232 lines)

5-minute setup guide for client development:

**Quick Setup**:
1. Install dependencies: `npm install`
2. Build shared libraries: `npm run build:shared && npm run build:shared-ui`
3. Choose client: `npm run dev:web` or `npm run dev:vscode`

**Development Workflow**:
- Watch mode for hot reload
- Shared library changes propagate automatically
- Platform-specific debugging

**Troubleshooting**:
- Missing module errors
- npm install failures
- TypeScript configuration issues

---

### Debug Client Complete
**File**: `docs/DEBUG-CLIENT-COMPLETE.md` (428 lines)

Standalone Electron debug application implementation:

**Features**:
- **Real-time Log Streaming**: Live server logs via SSE
- **Professional UI**: Dark theme, syntax highlighting, filters
- **Auto-reconnection**: Monitors server availability
- **Advanced Filtering**: By level (D/I/W/E) and text search
- **Statistics**: Log counts, connection status

**Architecture**:
- Server-side logging API (`/api/logs`, SSE streaming)
- Client-side `CronusClient` integration
- React `LogViewer` component (shared-ui)
- VSCode extension webview panel

**Components**:
- Server REST endpoints for log retrieval
- SSE broadcasting for real-time updates
- Reusable React components
- Electron desktop wrapper

---

### Configuration
**File**: `docs/configuration.md` (738 lines)

Complete configuration reference for both operational modes:

**Configuration Files**:
- `config.json`: Server and agent settings
- `state.json`: Runtime agent state
- `tasks/*.json`: Task management
- `context/history.jsonl`: Conversation logs

**Single-Agent Structure**:
- Location: `<repo-root>/.cronus/`
- Lock file for mutual exclusion
- Direct repository access

**Multi-Agent Structure**:
- Location: `<workspace>/.cronus/`
- Per-agent isolated directories
- Shared resources (cache, templates)
- Comparison and metrics data

**Schemas**:
- JSON schemas for all config files
- Environment variable support
- CLI commands for configuration

---

### Docker Build Fixes
**File**: `docs/DOCKER-BUILD-FIXES.md`

Build system troubleshooting and solutions:

**Issues Resolved**:
- Missing TypeScript dependencies
- Workspace protocol incompatibility
- File-based package references
- CronusClient API mismatches
- Event listener type errors

**Build Script Flow**:
1. Install shared dependencies
2. Build `@cronus/shared`
3. Install shared-ui dependencies
4. Build `@cronus/shared-ui`
5. Install debug client dependencies
6. Build debug client

**Key Changes**:
- Changed `workspace:*` to `file:` protocol
- Fixed TypeScript configurations
- Updated API usage patterns
- Corrected event handler types

---

### Roadmap
**File**: `docs/development/ROADMAP.md` (113 lines)

Development phases and milestones:

**Phase 0: Core Architecture**
- Mode detection and initialization
- Configuration management system
- State persistence layer

**Phase 1: Single-Agent Mode**
- Agent lifecycle management
- File modification tracking
- Lock mechanism
- Task management system

**Phase 2: Multi-Agent Mode**
- Agent workspace isolation
- Repository cloning
- Parallel execution
- Comparison system

**Phase 3: Client Expansion**
- Web client enhancement
- CLI improvements
- VS Code extension

**Phase 4: Agent Capabilities**
- Advanced configuration
- Specialized agents (coder, formatter, debugger, tester, optimizer)
- Custom prompts and tools

**Phase 5: Advanced Features**
- Repository synchronization
- Agent collaboration
- Cloud integration

---

### Research Decisions
**File**: `docs/research-decisions.md` (746 lines)

Technology evaluations and architectural decisions:

**C++ Parsing Technology**:
- Tree-sitter: Fast structural parsing (current)
- Clang/LibTooling: Deep semantic analysis (planned)
- Hybrid approach: Speed + accuracy

**Vector Database**:
- Faiss: High-performance similarity search (chosen)
- ANNOY, Milvus, Weaviate: Evaluated alternatives
- Custom persistence layer design

**Code Embedding Models**:
- CodeBERT, GraphCodeBERT
- UniXcoder, StarCoder
- Evaluation criteria and selection

**Communication Protocols**:
- gRPC for high-performance services
- WebSocket + JSON-RPC for clients
- REST for simple endpoints

**RAG Architecture**:
- Chunking strategies
- Embedding generation
- Retrieval algorithms
- Context assembly

---

### Features
**File**: `docs/ideas/features.txt` (182 lines)

Implemented and planned features:

**Currently Implemented**:
- Terminal UI with chat, file tree, input areas
- Multiple LLM providers (OpenAI, Anthropic, DeepSeek)
- Code analysis via loreforge (Tree-sitter parsing)
- RAG pipeline with semantic search
- Configuration system (environment, file-based)
- Evaluation system (Exercism-based, multi-language)

**Planned Features**:
- Operational mode system (single/multi-agent)
- Configuration management (.cronus/ directories)
- State persistence and recovery
- Task tracking and management
- Multi-agent comparison tools
- Advanced code understanding (Clang integration)

---

### Implementation Summary
**File**: `docs/IMPLEMENTATION-SUMMARY.md` (220 lines)

Client shared libraries implementation details:

**Structure Created**:
- `@cronus/shared`: Platform-agnostic core library
- `@cronus/shared-ui`: React components and hooks
- VSCode extension with webview integration
- Monorepo with NPM workspaces

**Code Sharing**:
- API Client: 100% shared
- SSE Manager: 100% shared
- Data Models: 100% shared
- React Hooks: 95% shared
- UI Components: 90% shared
- **Overall**: 70-80% code reuse

**Files Created**:
- 10+ shared library modules
- 8+ shared UI components
- VSCode extension infrastructure
- Comprehensive documentation

---

### Standalone Debug Client
**File**: `docs/STANDALONE-DEBUG-CLIENT.md` (289 lines)

Electron desktop application implementation:

**Architecture**:
- Electron multi-process (main, preload, renderer)
- Secure IPC with contextBridge
- CronusClient integration for API/SSE
- Professional dark theme UI

**Features**:
- Standalone desktop app (no VSCode needed)
- Cross-platform (Linux, Windows, Mac)
- Real-time log streaming
- Auto-reconnection and monitoring
- Advanced filtering and search
- User controls (connect, disconnect, clear)
- DevTools support

**Quick Start**:
```bash
./run_local.sh -d  # Build and launch
```

**Technical Highlights**:
- Secure IPC pattern with preload script
- Event flow: Logger → RestAPI → SSE → CronusClient → IPC → Renderer
- Build system integration
- Comprehensive documentation

---

## Quick Links

- **Project Overview**: [project.md](project.md)
- **Source Code**: `server/cronus/`, `server/arbiterAI/`, `server/loreforge/`
- **Clients**: `clients/web/`, `clients/vscode/`, `clients/cli/`, `clients/debug/`
- **Configuration**: [configuration.md](configuration.md)
- **API Reference**: [api-documentation.md](api-documentation.md)
- **Development**: [development-workflow.md](development-workflow.md)

---

*Last Updated: January 4, 2026*
