# Current Project State

**Last Updated:** December 06, 2024
**Status:** Active Development / Alpha

## Executive Summary

The **Cronus** project is in an active development phase, building a sophisticated AI Coding Agent system. The architecture consists of a central agent server (`cronus`) supported by two core specialized libraries: `arbiterAI` (LLM abstraction) and `loreforge` (Codebase RAG & Analysis).

Significant progress has been made on the backend infrastructure, with `arbiterAI` reaching a mature state and `loreforge` completing its core RAG pipeline. The client-side is currently focused on CLI interactions, with web and VS Code extensions planned.

---

## 1. Main Repository: Cronus

The main repository acts as the orchestration layer and entry point for the agent system.

**Current Status:**
- **Server:** Initial implementation of the `cronus` server is in place, capable of routing basic requests.
- **CLI Client:** Functional.
  - Can connect to the server.
  - Supports sending user input.
  - Receives and displays agent responses via SSE (Server-Sent Events).
  - Wired to use `cpr` for network communication.
- **Build System:** CMake-based, integrated with `vcpkg` for dependency management.

**Immediate Next Steps:**
- Refine server-side event broadcasting.
- Expand agent capabilities beyond basic chat (integration with `loreforge` tools).
- Prepare for VS Code extension development.

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
- monitoring and logging improvements.

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

**Immediate Next Steps:**
- Finalize gRPC service implementation (~Phase 4).
- Implement semantic similarity search using the Faiss index.
- Future: Deep semantic analysis using Clang/LibTooling (Phase 5).

---

## Summary Table

| Component | Role | Current Version/Phase | Key Tech Stack |
| :--- | :--- | :--- | :--- |
| **Cronus** | Agent Orchestrator & Server | Alpha / Dev | C++, Crow, vcpkg |
| **ArbiterAI** | LLM Provider Interface | **Mature** (Phase 4) | C++, cpr, nlohmann/json |
| **Loreforge** | Code Understanding & RAG | **Beta** (Phase 3 Complete) | C++, Tree-sitter, Faiss |
| **CLI Client** | User Interface | Functional Prototype | C++, CLI11 |
