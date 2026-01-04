# Current Project State

**Last Updated:** January 03, 2026
**Status:** Active Development / Alpha

## Executive Summary

The **Cronus** project is in an active development phase, building a sophisticated AI Coding Agent system with dual operational modes. The architecture consists of a central agent server (`cronus`) supported by two core specialized libraries: `arbiterAI` (LLM abstraction) and `loreforge` (Codebase RAG & Analysis).

Cronus now supports two distinct operational modes:
1. **Single-Agent Mode**: For existing repositories with one AI assistant
2. **Multi-Agent Mode**: For parallel agent experimentation with isolated repository trees

Significant progress has been made on the backend infrastructure, with `arbiterAI` reaching a mature state and `loreforge` completing its core RAG pipeline. The client-side is currently focused on CLI interactions, with web and VS Code extensions planned.

---

## 1. Main Repository: Cronus

The main repository acts as the orchestration layer and entry point for the agent system.

**Current Status:**
- **Server:** Initial implementation of the `cronus` server is in place, capable of routing basic requests.
- **Operational Modes:**
  - **Single-Agent Mode**: Ready for implementation. Designed for existing repositories with `.cronus/` configuration.
  - **Multi-Agent Mode**: Architecture defined. Supports multiple agents with isolated repository trees under `.cronus/agents/`.
- **Configuration Management:**
  - Mode detection based on directory state (existing repo vs empty/multi-agent directory)
  - `.cronus/` directory structure for persistent state
  - Per-agent state tracking in multi-agent mode
- **CLI Client:** Functional.
  - Can connect to the server.
  - Supports sending user input.
  - Receives and displays agent responses via SSE (Server-Sent Events).
  - Wired to use `cpr` for network communication.
- **Build System:** CMake-based, integrated with `vcpkg` for dependency management.

**Immediate Next Steps:**
- Implement mode detection and initialization logic
- Create `.cronus/` directory structure management
- Implement state persistence for agent tracking
- Build multi-agent repository isolation
- Refine server-side event broadcasting
- Expand agent capabilities beyond basic chat (integration with `loreforge` tools)
- Prepare for VS Code extension development

---

## 2. Sub-Repository: ArbiterAI

`arbiterAI` is the C++ library responsible for interacting with various LLM providers. It is the most mature component of the system.

**Current Status:** **Phase 4 (Optimization & Advanced Features)**
- **Core Library:** Stable. Provides a unified `Provider` interface.
- **Supported Providers:**
  - OpenAI (GPT-3.5/4)
  - Anthropic (Claude)
  - DeepSeek
  - Meta Llama (via Ollama)
  - OpenRouter (Gateway to various models)
- **Features:**
  - Standardized chat completion interface.
  - HTTP client implementation using `cpr`.
  - JSON handling with `nlohmann/json`.
  - Robust error handling and configuration.
- **Testing:** Comprehensive GTest suite verifying all providers.

**Immediate Next Steps:**
- Performance optimization.
- Monitoring and logging improvements.

---

## 3. Sub-Repository: Loreforge

`loreforge` is the C++ library and server for Codebase RAG (Retrieval-Augmented Generation) and semantic analysis.

**Current Status:** **Phase 4 (gRPC Server & Advanced Capabilities)**
- **Completed (Phases 1-3):**
  - **Parsing:** Integrated `tree-sitter` for robust multi-language parsing (C++, Python, etc.).
  - **Vector Database:** Integrated `Faiss` for similarity search.
  - **Embeddings:** Uses `arbiterAI` to fetch embeddings for code chunks.
  - **RAG Pipeline:**
    - File watching and incremental updates.
    - Code chunking (semantic units like functions/classes).
    - Embedding generation and storage.
    - Filesystem-based caching for persistence.
- **In Progress:**
  - **gRPC Interface:** Defining `.proto` files for tools like `AnalyzeFunction`, `FindSimilarCode`.
  - **Advanced Queries:** Implementing semantic similarity search and call graph extraction.
  - **Multi-Agent Support**: Adapting to work with isolated repository trees in multi-agent mode.

**Immediate Next Steps:**
- Finalize gRPC service implementation (~Phase 4).
- Implement semantic similarity search using the Faiss index.
- Add support for multiple repository paths (multi-agent mode).
- Future: Deep semantic analysis using Clang/LibTooling (Phase 5).

---

## 4. Configuration and State Management

**Current Status:** **Design Complete / Implementation Pending**

### Single-Agent Mode
- **Location**: `.cronus/` in repository root
- **Structure**:
  - `config.json`: Server and agent configuration
  - `state.json`: Current agent state
  - `tasks/`: Task tracking
  - `context/`: Conversation history
  - `cache/`: Temporary files
- **State**: Each server restart loads single agent state

### Multi-Agent Mode
- **Location**: `.cronus/` in working directory
- **Structure**:
  - `config.json`: Global server configuration
  - `agents/<agent-id>/repo/`: Isolated repository copy per agent
  - `agents/<agent-id>/config.json`: Per-agent configuration
  - `agents/<agent-id>/state.json`: Per-agent state
  - `agents/<agent-id>/tasks/`: Per-agent task tracking
  - `agents/<agent-id>/context/`: Per-agent conversation history
  - `shared/`: Shared resources (cache, templates)
- **State**: Server tracks all agent states independently

**Immediate Next Steps:**
- Implement configuration file schemas
- Create directory initialization logic
- Build state persistence layer
- Implement mode detection algorithm

---

## Summary Table

| Component | Role | Current Version/Phase | Key Tech Stack |
| :--- | :--- | :--- | :--- |
| **Cronus** | Agent Orchestrator & Server | Alpha / Dev | C++, Crow, vcpkg |
| **Mode System** | Single/Multi-Agent Support | Design Complete | Configuration management |
| **ArbiterAI** | LLM Provider Interface | **Mature** (Phase 4) | C++, cpr, nlohmann/json |
| **Loreforge** | Code Understanding & RAG | **Beta** (Phase 3 Complete) | C++, Tree-sitter, Faiss |
| **CLI Client** | User Interface | Functional Prototype | C++, CLI11 |

---

## Next Major Milestones

1. **Configuration System Implementation** (2-3 weeks)
   - Mode detection
   - `.cronus/` directory management
   - State persistence

2. **Multi-Agent Repository Isolation** (2-3 weeks)
   - Repository cloning and management
   - Agent workspace isolation
   - Concurrent agent execution

3. **Web Client Development** (4-6 weeks)
   - React-based UI
   - Mode-aware interface
   - Real-time agent status display

4. **VS Code Extension** (3-4 weeks)
   - Reuse web client components
   - VS Code integration APIs
   - Workspace integration
