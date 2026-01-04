# Cronus Architecture: Dual-Mode Operation

**Last Updated:** January 03, 2026

## Overview

Cronus is designed with a flexible architecture that supports two distinct operational modes based on the directory context in which it is launched. This dual-mode design allows the system to serve both individual developers working with a single AI assistant and researchers/teams experimenting with multiple AI agents in parallel.

---

## Operational Modes

### 1. Single-Agent Mode

**Trigger Condition**: Cronus is started in an existing repository (detected by presence of `.git/` or other version control markers).

#### Characteristics

- **One Agent, One Repository**: A single AI agent works on the existing codebase
- **Configuration Location**: `.cronus/` directory at repository root
- **Mutual Exclusion**: Only one Cronus instance can modify the repository at a time
- **State Persistence**: Agent state, context, and configuration survive server restarts
- **Use Cases**:
  - Individual developer with AI coding assistant
  - Single project workflow
  - Continuous development with persistent context

#### Directory Structure

```
my-project/                    # Existing repository root
├── .git/                      # Version control
├── .cronus/                   # Cronus configuration (added)
│   ├── config.json            # Agent and server configuration
│   ├── state.json             # Current agent state
│   ├── lock                   # Lock file to prevent concurrent access
│   ├── tasks/                 # Task management
│   │   ├── todo.json          # Pending tasks
│   │   ├── in_progress.json   # Active tasks
│   │   └── done.json          # Completed tasks
│   ├── context/               # Conversation and context
│   │   ├── history.jsonl      # Conversation history
│   │   ├── embeddings/        # Context embeddings
│   │   └── summaries/         # Session summaries
│   ├── cache/                 # Temporary files
│   │   ├── loreforge/         # Code analysis cache
│   │   └── embeddings/        # Embedding cache
│   └── logs/                  # Agent logs
│       └── agent.log
├── src/                       # Application source
└── ...                        # Project files
```

#### Agent State

```json
{
  "mode": "single-agent",
  "agent": {
    "id": "primary",
    "status": "active|idle|error",
    "model": "gpt-4",
    "created_at": "2026-01-03T10:00:00Z",
    "last_active": "2026-01-03T12:30:00Z",
    "working_directory": "/path/to/my-project"
  },
  "repository": {
    "root": "/path/to/my-project",
    "branch": "main",
    "last_commit": "abc123..."
  },
  "session": {
    "messages": 42,
    "files_modified": 15,
    "tasks_completed": 7
  }
}
```

---

### 2. Multi-Agent Mode

**Trigger Conditions**: 
- Cronus is started in an **empty directory**, OR
- Directory contains `.cronus/multi-agent.marker` file

#### Characteristics

- **Multiple Agents, Isolated Repositories**: Each agent works in its own repository copy
- **Configuration Location**: `.cronus/` directory in the working directory
- **Parallel Execution**: Multiple agents can run simultaneously without conflicts
- **Independent State**: Each agent maintains separate state, context, and configuration
- **Repository Isolation**: Each agent has a complete copy of the repository
- **Use Cases**:
  - Agent comparison and evaluation
  - Parallel experimentation with different models/prompts
  - Multi-agent collaboration research
  - A/B testing of agent configurations

#### Directory Structure

```
workspace/                      # Multi-agent workspace root
├── .cronus/                    # Global Cronus configuration
│   ├── multi-agent.marker      # Indicates multi-agent mode
│   ├── config.json             # Global server configuration
│   ├── shared/                 # Resources shared across agents
│   │   ├── cache/              # Shared cache (embeddings, etc.)
│   │   ├── templates/          # Shared prompt templates
│   │   └── models/             # Model configurations
│   ├── logs/                   # Server logs
│   │   └── server.log
│   └── comparison/             # Agent comparison data
│       ├── metrics.json
│       └── reports/
│
├── agents/                     # Agent workspace directories
│   ├── agent-alpha/
│   │   ├── .cronus/            # Agent-alpha's configuration
│   │   │   ├── config.json     # Agent-specific configuration
│   │   │   ├── state.json      # Agent state
│   │   │   ├── tasks/          # Agent's task tracking
│   │   │   │   ├── todo.json
│   │   │   │   ├── in_progress.json
│   │   │   │   └── done.json
│   │   │   ├── context/        # Agent's conversation history
│   │   │   │   ├── history.jsonl
│   │   │   │   └── summaries/
│   │   │   ├── cache/          # Agent's cache
│   │   │   │   └── loreforge/
│   │   │   └── logs/
│   │   │       └── agent.log
│   │   ├── .git/               # Agent's git repository
│   │   ├── src/                # Agent's source code
│   │   └── ...                 # Other repository files
│   │
│   ├── agent-beta/
│   │   ├── .cronus/            # Agent-beta's configuration
│   │   ├── .git/               # Agent's git repository
│   │   ├── src/
│   │   └── ...
│   │
│   └── agent-gamma/
│       ├── .cronus/            # Agent-gamma's configuration
│       ├── .git/
│       └── ...
│
└── source-repo/                # (Optional) Original source repository
```

#### Multi-Agent State

```json
{
  "mode": "multi-agent",
  "workspace": {
    "root": "/path/to/workspace",
    "created_at": "2026-01-03T10:00:00Z"
  },
  "agents": [
    {
      "id": "agent-alpha",
      "status": "active",
      "model": "gpt-4",
      "workspace_path": "agents/agent-alpha",
      "created_at": "2026-01-03T10:00:00Z",
      "last_active": "2026-01-03T12:30:00Z",
      "stats": {
        "messages": 28,
        "files_modified": 8,
        "tasks_completed": 3
      }
    },
    {
      "id": "agent-beta",
      "status": "idle",
      "model": "claude-3-opus",
      "workspace_path": "agents/agent-beta",
      "created_at": "2026-01-03T10:05:00Z",
      "last_active": "2026-01-03T11:45:00Z",
      "stats": {
        "messages": 35,
        "files_modified": 12,
        "tasks_completed": 5
      }
    }
  ],
  "source_repository": "/path/to/source-repo"
}
```

---

## Mode Detection Algorithm

```cpp
enum class OperationalMode {
    SingleAgent,
    MultiAgent
};

OperationalMode detectMode(const std::filesystem::path& workingDir) {
    // Check for multi-agent marker
    if (std::filesystem::exists(workingDir / ".cronus" / "multi-agent.marker")) {
        return OperationalMode::MultiAgent;
    }
    
    // Check for existing .cronus directory
    if (std::filesystem::exists(workingDir / ".cronus")) {
        // Load config to determine mode
        auto config = loadConfig(workingDir / ".cronus" / "config.json");
        return config["mode"] == "multi-agent" 
            ? OperationalMode::MultiAgent 
            : OperationalMode::SingleAgent;
    }
    
    // Check for version control (indicates existing repository)
    if (std::filesystem::exists(workingDir / ".git") ||
        std::filesystem::exists(workingDir / ".svn") ||
        std::filesystem::exists(workingDir / ".hg")) {
        return OperationalMode::SingleAgent;
    }
    
    // Check if directory is empty or nearly empty
    int fileCount = 0;
    for (const auto& entry : std::filesystem::directory_iterator(workingDir)) {
        fileCount++;
        if (fileCount > 5) break;  // Not empty
    }
    
    if (fileCount <= 1) {
        // Empty or nearly empty directory -> prompt for mode
        // Default to multi-agent for empty directories
        return OperationalMode::MultiAgent;
    }
    
    // Default to single-agent for directories with content
    return OperationalMode::SingleAgent;
}
```

---

## State Management

### Single-Agent State Lifecycle

```
┌─────────────────────────────────────────────────────────────┐
│ Server Start                                                 │
└────────────┬────────────────────────────────────────────────┘
             │
             ▼
      ┌─────────────┐
      │ Detect Mode │
      └──────┬──────┘
             │
             ▼ (Single-Agent)
      ┌──────────────────┐
      │ Load .cronus/    │
      │ state.json       │
      └──────┬───────────┘
             │
             ├─► (No state) → Initialize new agent
             │
             └─► (State exists) → Restore agent
                      │
                      ▼
               ┌──────────────┐
               │ Agent Active │
               └──────┬───────┘
                      │
                      ├─► Handle requests
                      ├─► Modify files
                      ├─► Update state
                      │
                      ▼
               ┌──────────────┐
               │ Persist State│ (Periodic + on change)
               └──────┬───────┘
                      │
                      ▼
               ┌──────────────┐
               │ Server Stop  │
               └──────────────┘
```

### Multi-Agent State Lifecycle

```
┌─────────────────────────────────────────────────────────────┐
│ Server Start                                                 │
└────────────┬────────────────────────────────────────────────┘
             │
             ▼
      ┌─────────────┐
      │ Detect Mode │
      └──────┬──────┘
             │
             ▼ (Multi-Agent)
      ┌────────────────────┐
      │ Load .cronus/      │
      │ agents/*/state.json│
      └──────┬─────────────┘
             │
             ├─► For each agent directory:
             │    ├─► Load agent state
             │    ├─► Verify repository integrity
             │    └─► Initialize agent
             │
             ▼
      ┌─────────────────┐
      │ All Agents      │
      │ Active/Restored │
      └────────┬────────┘
               │
               ├─► Agent 1 ──┐
               ├─► Agent 2 ──┤ (Parallel execution)
               └─► Agent N ──┘
                      │
                      ▼
               ┌──────────────────┐
               │ Periodic State   │
               │ Persistence      │
               └──────┬───────────┘
                      │
                      ▼
               ┌──────────────┐
               │ Server Stop  │
               │ Save all     │
               │ agent states │
               └──────────────┘
```

---

## Repository Isolation (Multi-Agent)

### Repository Initialization

When a new agent is created in multi-agent mode:

```cpp
void createAgentRepository(
    const std::string& agentId,
    const std::filesystem::path& workspaceRoot,
    const std::filesystem::path& sourceRepo
) {
    auto agentDir = workspaceRoot / "agents" / agentId;
    auto cronusDir = agentDir / ".cronus";
    
    // Create agent directory structure
    std::filesystem::create_directories(agentDir);
    std::filesystem::create_directories(cronusDir / "tasks");
    std::filesystem::create_directories(cronusDir / "context");
    std::filesystem::create_directories(cronusDir / "cache");
    std::filesystem::create_directories(cronusDir / "logs");
    
    // Clone or copy source repository into agent's workspace
    if (!sourceRepo.empty()) {
        // Git clone preserves history
        system(fmt::format("git clone {} {}", 
            sourceRepo.string(), agentDir.string()).c_str());
    } else {
        // Create empty repository
        system(fmt::format("cd {} && git init", 
            agentDir.string()).c_str());
    }
    
    // Initialize agent configuration
    initializeAgentConfig(cronusDir, agentId);
}
```

### Repository Synchronization

Agents can optionally sync with a source repository:

```cpp
void syncAgentRepository(
    const std::string& agentId,
    const std::filesystem::path& workspaceRoot,
    SyncMode mode  // Pull, Push, or Both
) {
    auto agentDir = workspaceRoot / "agents" / agentId;
    
    if (mode == SyncMode::Pull || mode == SyncMode::Both) {
        // Pull changes from source
        system(fmt::format("cd {} && git pull origin main", 
            agentDir.string()).c_str());
    }
    
    if (mode == SyncMode::Push || mode == SyncMode::Both) {
        // Push agent's changes to a branch
        auto branchName = fmt::format("agent-{}-changes", agentId);
        system(fmt::format(
            "cd {} && git checkout -b {} && git push origin {}", 
            agentDir.string(), branchName, branchName).c_str());
    }
}
```

---

## Configuration Schema

### Single-Agent `config.json`

```json
{
  "version": "1.0",
  "mode": "single-agent",
  "server": {
    "host": "localhost",
    "port": 9000,
    "api_path": "/api"
  },
  "agent": {
    "id": "primary",
    "name": "Primary Agent",
    "model": {
      "provider": "openai",
      "name": "gpt-4",
      "temperature": 0.7,
      "max_tokens": 4096
    },
    "prompts": {
      "system": "You are an expert coding assistant...",
      "coder": "prompts/coder.txt",
      "reviewer": "prompts/reviewer.txt"
    },
    "tools": [
      "code_search",
      "file_edit",
      "terminal",
      "test_runner"
    ]
  },
  "repository": {
    "root": ".",
    "watch_patterns": ["src/**/*.cpp", "include/**/*.h"],
    "ignore_patterns": ["build/**", ".git/**"]
  },
  "loreforge": {
    "enabled": true,
    "cache_dir": ".cronus/cache/loreforge"
  }
}
```

### Multi-Agent `config.json`

```json
{
  "version": "1.0",
  "mode": "multi-agent",
  "server": {
    "host": "localhost",
    "port": 9000,
    "api_path": "/api"
  },
  "workspace": {
    "root": ".",
    "source_repository": "/path/to/source/repo",
    "auto_sync": false
  },
  "agents": [
    {
      "id": "agent-alpha",
      "name": "Alpha Agent (GPT-4)",
      "enabled": true,
      "workspace_path": "agents/agent-alpha",
      "model": {
        "provider": "openai",
        "name": "gpt-4",
        "temperature": 0.7
      }
    },
    {
      "id": "agent-beta",
      "name": "Beta Agent (Claude)",
      "enabled": true,
      "workspace_path": "agents/agent-beta",
      "model": {
        "provider": "anthropic",
        "name": "claude-3-opus-20240229",
        "temperature": 0.8
      }
    }
  ],
  "shared": {
    "cache_dir": ".cronus/shared/cache",
    "templates_dir": ".cronus/shared/templates"
  },
  "comparison": {
    "enabled": true,
    "metrics": ["correctness", "performance", "code_quality"],
    "output_dir": ".cronus/comparison"
  }
}
```

---

## API Differences

### Single-Agent Mode Endpoints

```
GET  /api/status              # Get agent status
POST /api/chat                # Send message to agent
GET  /api/tasks               # Get task list
POST /api/tasks               # Create task
GET  /api/context             # Get conversation context
POST /api/files/edit          # Request file edit
```

### Multi-Agent Mode Endpoints

```
GET  /api/status                    # Get all agents' status
GET  /api/agents                    # List all agents
POST /api/agents                    # Create new agent
GET  /api/agents/{id}/status        # Get specific agent status
POST /api/agents/{id}/chat          # Send message to specific agent
GET  /api/agents/{id}/tasks         # Get agent's tasks
POST /api/agents/{id}/files/edit    # Request file edit by agent
GET  /api/comparison                # Get agent comparison metrics
```

---

## Benefits of Dual-Mode Architecture

### Single-Agent Mode Benefits
1. **Simplicity**: No complexity of managing multiple repositories
2. **Performance**: Direct file access, no repository isolation overhead
3. **Integration**: Works seamlessly with existing projects
4. **Familiarity**: Traditional single-assistant workflow

### Multi-Agent Mode Benefits
1. **Experimentation**: Test multiple approaches in parallel
2. **Comparison**: Direct comparison of different models/configurations
3. **Safety**: Isolated repositories prevent conflicts
4. **Research**: Enables multi-agent collaboration studies
5. **Evaluation**: Built-in metrics for agent performance comparison

---

## Client Architecture

**Last Updated:** January 04, 2026

### Overview

Cronus clients are organized as a monorepo with shared libraries to maximize code reuse across different platforms (web, VSCode, desktop, CLI).

### Monorepo Structure

```
clients/
├── shared/          # @cronus/shared - Core API client and utilities
├── shared-ui/       # @cronus/shared-ui - React components and hooks
├── web/             # @cronus/web - React web application
├── vscode/          # cronus-vscode - VSCode extension
└── cli/             # CLI client
```

### Shared Libraries

#### @cronus/shared

Core TypeScript library providing platform-agnostic functionality:
- **CronusClient**: Unified API client with automatic reconnection
- **SSEManager**: Server-Sent Events manager with exponential backoff
- **Type Definitions**: Shared data models (Message, FileTree, AgentState, etc.)
- **Logger**: Configurable logging utility

Key Features:
- Auto-reconnection with exponential backoff
- Event-driven architecture using EventEmitter
- Health check monitoring
- Connection status tracking
- ~100% code sharing across all clients

#### @cronus/shared-ui

React UI library providing reusable components and hooks:
- **Custom Hooks**: `useCronusConnection`, `useFileTree`, `useMessages`
- **UI Components**: `ConnectionStatus`, etc.
- **Platform-agnostic**: Works in browser, VSCode webview, and Electron

Key Features:
- React 18+ support
- TypeScript with full type safety
- CSS with VSCode theming support
- ~90% code sharing between web and VSCode clients

### Client Applications

#### Web Client (@cronus/web)

React-based web application accessible via browser.

**Features:**
- FlexLayout for flexible UI panels
- Real-time updates via SSE
- File tree explorer
- Chat interface
- Code editor

**Technology Stack:**
- React 18
- TypeScript
- FlexLayout React
- @cronus/shared + @cronus/shared-ui

#### VSCode Extension (cronus-vscode)

Native VSCode extension for debugging and monitoring Cronus.

**Features:**
- Always-on debug panel
- Auto-reconnection to server
- System tray integration
- Native VSCode theming
- Command palette commands

**Technology Stack:**
- VSCode Extension API
- Webview for UI (React)
- @cronus/shared for server communication
- @cronus/shared-ui for webview components

**Architecture:**
```
┌─────────────────────────────────────┐
│   VSCode Extension Host             │
│   ├─ Extension.ts                   │
│   │  - CronusClient (@cronus/shared)│
│   │  - Command handlers             │
│   │  - Configuration management     │
│   └─ Bridge to Webview              │
│      ↓                               │
├─────────────────────────────────────┤
│   Webview (React)                   │
│   ├─ UI Components (@cronus/shared-ui)│
│   ├─ ConnectionStatus               │
│   ├─ LogViewer                      │
│   └─ Message bridge to extension    │
└─────────────────────────────────────┘
```

#### CLI Client

Command-line interface for scripting and automation.

**Features:**
- Non-interactive mode for scripts
- File operations
- Query codebase
- Agent management

### Code Sharing Breakdown

| Component | Web | VSCode | CLI | Shared % |
|-----------|-----|--------|-----|----------|
| API Client | ✓ | ✓ | ✓ | **100%** |
| SSE Manager | ✓ | ✓ | ✓ | **100%** |
| Data Models | ✓ | ✓ | ✓ | **100%** |
| Business Logic | ✓ | ✓ | ✓ | **100%** |
| React Hooks | ✓ | ✓ (webview) | ✗ | **95%** |
| UI Components | ✓ | ✓ (webview) | ✗ | **90%** |
| Layout/Chrome | ✗ | ✗ | ✗ | **0%** |
| Platform Integration | ✗ | ✗ | ✗ | **0%** |

**Overall: ~70-80% code sharing**

### Development Workflow

**NPM Workspaces:**
The project uses NPM workspaces for monorepo management:
- Single `node_modules` at root
- Workspace references using `workspace:*` protocol
- Coordinated build process

**Build Process:**
```bash
# Build shared libraries first
npm run build:shared
npm run build:shared-ui

# Then build clients
npm run build:web
npm run build:vscode
```

**Development:**
```bash
# Watch mode for live development
npm run watch:shared        # Terminal 1
npm run watch:shared-ui     # Terminal 2
npm run dev:web            # Terminal 3 - or -
npm run dev:vscode         # Terminal 3 - or -
```

### Communication Flow

```
┌─────────────────────────────────────────────────────────────┐
│                    Cronus Server (C++)                       │
│              REST API + SSE (Port 9000)                      │
└─────────────────────────────────────────────────────────────┘
                           │
                ┌──────────┴──────────┐
                │   @cronus/shared    │  ← Shared Core Library
                │  - CronusClient     │
                │  - SSEManager       │
                │  - Data Models      │
                └──────────┬──────────┘
                           │
        ┌──────────────────┼──────────────────┐
        │                  │                  │
┌───────▼────────┐  ┌──────▼──────┐  ┌───────▼────────┐
│  @cronus/web   │  │ cronus-     │  │ @cronus/cli    │
│   (React)      │  │  vscode     │  │                │
│                │  │             │  │                │
│ Uses:          │  │ Uses:       │  │ Uses:          │
│ - shared       │  │ - shared    │  │ - shared       │
│ - shared-ui    │  │ - shared-ui │  │                │
└────────────────┘  │  (Webview)  │  └────────────────┘
                    │             │
                    │ Extension   │
                    │ Host API    │
                    └─────────────┘
```

### Benefits of This Architecture

1. **Consistency**: Same API client behavior across all platforms
2. **Maintainability**: Fix bugs once, benefit everywhere
3. **Type Safety**: TypeScript types shared across all clients
4. **Faster Development**: Reuse components and logic
5. **Testing**: Test shared code once
6. **Platform-Specific Features**: Each client can add unique features while sharing core logic

---

## Future Enhancements

1. **Hybrid Mode**: Single repository with multiple agents using branches
2. **Agent Communication**: Agents can share insights in multi-agent mode
3. **Dynamic Mode Switching**: Convert between modes without restarting
4. **Cloud Integration**: Sync multi-agent workspaces to cloud storage
5. **Agent Orchestration**: Coordinated multi-agent workflows (e.g., one agent writes, another reviews)
6. **Electron Debug Client**: Standalone desktop application with full system access
7. **Mobile Client**: React Native app using @cronus/shared
8. **Browser Extension**: Chrome/Firefox extension for web integration
