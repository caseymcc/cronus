# Cronus Configuration Reference

**Last Updated:** January 03, 2026

This document provides a complete reference for Cronus configuration files and directory structures in both operational modes.

---

## Table of Contents

1. [Configuration Directory Overview](#configuration-directory-overview)
2. [Single-Agent Mode Configuration](#single-agent-mode-configuration)
3. [Multi-Agent Mode Configuration](#multi-agent-mode-configuration)
4. [Configuration File Schemas](#configuration-file-schemas)
5. [State File Schemas](#state-file-schemas)
6. [Environment Variables](#environment-variables)
7. [CLI Commands](#cli-commands)

---

## Configuration Directory Overview

Cronus stores all configuration, state, and runtime data in a `.cronus/` directory. The structure and location of this directory depends on the operational mode.

### Quick Reference

| Mode | Location | Purpose |
|------|----------|---------|
| **Single-Agent** | `<repo-root>/.cronus/` | Configuration for one agent working in the repository |
| **Multi-Agent** | `<workspace>/.cronus/` | Configuration for multiple agents with isolated repositories |

---

## Single-Agent Mode Configuration

### Directory Structure

```
my-project/                      # Your repository root
├── .git/                        # Git repository
├── .cronus/                     # Cronus configuration
│   ├── config.json              # Main configuration file
│   ├── state.json               # Current agent state
│   ├── lock                     # Lock file (prevents concurrent access)
│   │
│   ├── tasks/                   # Task management
│   │   ├── todo.json            # Tasks not started
│   │   ├── in_progress.json     # Tasks being worked on
│   │   └── done.json            # Completed tasks
│   │
│   ├── context/                 # Conversation and context
│   │   ├── history.jsonl        # Conversation history (JSON Lines)
│   │   ├── summaries/           # Session summaries
│   │   │   ├── session-001.json
│   │   │   └── session-002.json
│   │   └── embeddings/          # Context embeddings
│   │       └── context.faiss
│   │
│   ├── cache/                   # Cached data
│   │   ├── loreforge/           # Code analysis cache
│   │   │   ├── symbols.db
│   │   │   └── embeddings.faiss
│   │   └── embeddings/          # Model embeddings cache
│   │
│   ├── logs/                    # Log files
│   │   ├── agent.log            # Agent activity log
│   │   ├── server.log           # Server log
│   │   └── errors.log           # Error log
│   │
│   └── prompts/                 # Custom prompts (optional)
│       ├── coder.txt
│       ├── reviewer.txt
│       └── debugger.txt
│
├── src/                         # Your application source
└── ...                          # Other project files
```

### File Purposes

| File/Directory | Purpose | Format |
|----------------|---------|--------|
| `config.json` | Agent and server configuration | JSON |
| `state.json` | Current agent state (active/idle/error) | JSON |
| `lock` | Prevents concurrent Cronus instances | Text |
| `tasks/*.json` | Task tracking and management | JSON |
| `context/history.jsonl` | Complete conversation history | JSON Lines |
| `context/summaries/` | Summarized sessions for context | JSON |
| `cache/loreforge/` | Code understanding database | Binary |
| `logs/*.log` | Activity and error logging | Text |
| `prompts/*.txt` | Custom agent prompts | Text |

---

## Multi-Agent Mode Configuration

### Directory Structure

```
workspace/                        # Multi-agent workspace root
├── .cronus/                      # Global Cronus configuration
│   ├── multi-agent.marker        # Indicates multi-agent mode
│   ├── config.json               # Global server configuration
│   │
│   ├── shared/                   # Resources shared across agents
│   │   ├── cache/                # Shared cache (embeddings, etc.)
│   │   │   └── embeddings/
│   │   ├── templates/            # Shared prompt templates
│   │   │   ├── coder.txt
│   │   │   └── reviewer.txt
│   │   └── models/               # Model configurations
│   │       └── model-configs.json
│   │
│   ├── comparison/               # Agent comparison data
│   │   ├── metrics.json          # Comparison metrics
│   │   ├── reports/              # Comparison reports
│   │   │   ├── report-001.html
│   │   │   └── report-002.json
│   │   └── diffs/                # Code diffs between agents
│   │
│   └── logs/                     # Server logs
│       ├── server.log
│       └── orchestrator.log
│
├── agents/                       # Agent workspace directories
│   ├── agent-alpha/
│   │   ├── .cronus/              # Agent-alpha's configuration
│   │   │   ├── config.json       # Agent-specific config
│   │   │   ├── state.json        # Agent state
│   │   │   ├── tasks/            # Agent's tasks
│   │   │   │   ├── todo.json
│   │   │   │   ├── in_progress.json
│   │   │   │   └── done.json
│   │   │   ├── context/          # Agent's context
│   │   │   │   ├── history.jsonl
│   │   │   │   └── summaries/
│   │   │   ├── cache/            # Agent's cache
│   │   │   │   └── loreforge/
│   │   │   └── logs/             # Agent's logs
│   │   │       ├── agent.log
│   │   │       └── errors.log
│   │   ├── .git/                 # Agent's git repository
│   │   ├── src/                  # Agent's source code
│   │   └── ...                   # Other repository files
│   │
│   ├── agent-beta/
│   │   ├── .cronus/              # Agent-beta's configuration
│   │   ├── .git/
│   │   ├── src/
│   │   └── ...
│   │
│   └── agent-gamma/
│       ├── .cronus/
│       └── ...
│
└── source-repo/                  # (Optional) Original source repository
```

### File Purposes

| File/Directory | Purpose | Format |
|----------------|---------|--------|
| `multi-agent.marker` | Indicates multi-agent mode | Empty file |
| `config.json` | Global server and workspace config | JSON |
| `agents/<id>/` | Agent's workspace directory | Directory |
| `agents/<id>/.cronus/` | Agent's configuration directory | Directory |
| `agents/<id>/.cronus/config.json` | Per-agent configuration | JSON |
| `agents/<id>/.cronus/state.json` | Per-agent state | JSON |
| `agents/<id>/.git/` | Agent's git repository | Git repo |
| `.cronus/shared/cache/` | Shared data (embeddings, models) | Binary |
| `.cronus/shared/templates/` | Shared prompts and templates | Text |
| `.cronus/comparison/metrics.json` | Agent performance metrics | JSON |
| `.cronus/comparison/reports/` | Comparison reports | HTML/JSON |

---

## Configuration File Schemas

### Single-Agent `config.json`

```json
{
  "version": "1.0",
  "mode": "single-agent",
  
  "server": {
    "host": "localhost",
    "port": 9000,
    "api_path": "/api",
    "enable_cors": true,
    "log_level": "info"
  },
  
  "agent": {
    "id": "primary",
    "name": "Primary Agent",
    "enabled": true,
    
    "model": {
      "provider": "openai",
      "name": "gpt-4",
      "api_key": "${OPENAI_API_KEY}",
      "api_base": "https://api.openai.com/v1",
      "temperature": 0.7,
      "max_tokens": 4096,
      "top_p": 1.0
    },
    
    "prompts": {
      "system": "You are an expert coding assistant specialized in helping developers write, refactor, and debug code.",
      "coder": "prompts/coder.txt",
      "reviewer": "prompts/reviewer.txt",
      "debugger": "prompts/debugger.txt",
      "tester": "prompts/tester.txt"
    },
    
    "tools": [
      "code_search",
      "file_read",
      "file_edit",
      "terminal",
      "test_runner",
      "linter"
    ],
    
    "capabilities": {
      "max_file_size": 1048576,
      "max_context_messages": 50,
      "auto_save": true,
      "auto_format": true
    }
  },
  
  "repository": {
    "root": ".",
    "watch_patterns": [
      "src/**/*.cpp",
      "src/**/*.h",
      "include/**/*.h",
      "tests/**/*.cpp"
    ],
    "ignore_patterns": [
      "build/**",
      ".git/**",
      "*.o",
      "*.a"
    ],
    "auto_reload": true
  },
  
  "loreforge": {
    "enabled": true,
    "server": "localhost:50051",
    "cache_dir": ".cronus/cache/loreforge",
    "embedding_model": "text-embedding-3-small",
    "chunk_size": 512,
    "chunk_overlap": 50
  },
  
  "task_management": {
    "enabled": true,
    "auto_track": true,
    "todo_dir": ".cronus/tasks"
  },
  
  "logging": {
    "level": "info",
    "log_dir": ".cronus/logs",
    "max_size_mb": 100,
    "max_files": 10
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
    "api_path": "/api",
    "enable_cors": true,
    "log_level": "info",
    "max_concurrent_agents": 10
  },
  
  "workspace": {
    "root": ".",
    "source_repository": "/path/to/source/repo",
    "auto_sync": false,
    "sync_interval_minutes": 30
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
        "api_key": "${OPENAI_API_KEY}",
        "temperature": 0.7,
        "max_tokens": 4096
      },
      
      "prompts": {
        "system": ".cronus/shared/templates/coder.txt"
      },
      
      "tools": ["code_search", "file_edit", "terminal"]
    },
    
    {
      "id": "agent-beta",
      "name": "Beta Agent (Claude)",
      "enabled": true,
      "workspace_path": "agents/agent-beta",
      
      "model": {
        "provider": "anthropic",
        "name": "claude-3-opus-20240229",
        "api_key": "${ANTHROPIC_API_KEY}",
        "temperature": 0.8,
        "max_tokens": 4096
      },
      
      "prompts": {
        "system": ".cronus/shared/templates/coder.txt"
      },
      
      "tools": ["code_search", "file_edit", "terminal"]
    },
    
    {
      "id": "agent-gamma",
      "name": "Gamma Agent (DeepSeek)",
      "enabled": true,
      "workspace_path": "agents/agent-gamma",
      
      "model": {
        "provider": "deepseek",
        "name": "deepseek-coder",
        "api_key": "${DEEPSEEK_API_KEY}",
        "temperature": 0.6,
        "max_tokens": 4096
      },
      
      "prompts": {
        "system": ".cronus/shared/templates/coder.txt"
      },
      
      "tools": ["code_search", "file_edit", "terminal"]
    }
  ],
  
  "shared": {
    "cache_dir": ".cronus/shared/cache",
    "templates_dir": ".cronus/shared/templates",
    "models_dir": ".cronus/shared/models",
    
    "loreforge": {
      "enabled": true,
      "server": "localhost:50051",
      "embedding_model": "text-embedding-3-small"
    }
  },
  
  "comparison": {
    "enabled": true,
    "metrics": [
      "correctness",
      "performance",
      "code_quality",
      "test_coverage",
      "response_time"
    ],
    "output_dir": ".cronus/comparison",
    "auto_generate_reports": true
  },
  
  "logging": {
    "level": "info",
    "log_dir": ".cronus/logs",
    "max_size_mb": 100,
    "max_files": 10,
    "per_agent_logs": true
  }
}
```

### Per-Agent `config.json` (Multi-Agent Mode)

Located at `agents/<agent-id>/.cronus/config.json`:

```json
{
  "agent_id": "agent-alpha",
  "created_at": "2026-01-03T10:00:00Z",
  "updated_at": "2026-01-03T12:30:00Z",
  
  "model": {
    "provider": "openai",
    "name": "gpt-4",
    "temperature": 0.7,
    "max_tokens": 4096
  },
  
  "repository": {
    "root": ".",
    "source": "/path/to/source/repo",
    "branch": "main",
    "last_sync": "2026-01-03T10:00:00Z"
  },
  
  "prompts": {
    "system": "../../.cronus/shared/templates/coder.txt",
    "custom_instructions": "Focus on performance optimization"
  },
  
  "capabilities": {
    "max_file_size": 1048576,
    "max_context_messages": 50,
    "auto_save": true,
    "auto_format": true
  },
  
  "statistics": {
    "messages_sent": 142,
    "files_modified": 28,
    "tasks_completed": 12,
    "uptime_hours": 3.5
  }
}
```

---

## State File Schemas

### Single-Agent `state.json`

```json
{
  "version": "1.0",
  "mode": "single-agent",
  "last_updated": "2026-01-03T12:30:00Z",
  
  "agent": {
    "id": "primary",
    "status": "active",
    "created_at": "2026-01-03T10:00:00Z",
    "last_active": "2026-01-03T12:30:00Z",
    "working_directory": "/path/to/my-project"
  },
  
  "repository": {
    "root": "/path/to/my-project",
    "branch": "main",
    "last_commit": "abc123def456",
    "uncommitted_changes": true,
    "files_tracked": 156
  },
  
  "session": {
    "id": "session-42",
    "started_at": "2026-01-03T10:00:00Z",
    "messages": 42,
    "files_modified": 15,
    "tasks_completed": 7,
    "current_task": "Implement authentication module"
  },
  
  "context": {
    "message_count": 42,
    "context_size_tokens": 12543,
    "active_files": [
      "src/auth/login.cpp",
      "include/auth/auth.h"
    ]
  },
  
  "loreforge": {
    "indexed_files": 156,
    "last_index_update": "2026-01-03T12:00:00Z",
    "cache_size_mb": 45.2
  }
}
```

### Multi-Agent `state.json` (Global)

```json
{
  "version": "1.0",
  "mode": "multi-agent",
  "last_updated": "2026-01-03T12:30:00Z",
  
  "workspace": {
    "root": "/path/to/workspace",
    "created_at": "2026-01-03T10:00:00Z",
    "agent_count": 3
  },
  
  "agents": [
    {
      "id": "agent-alpha",
      "status": "active",
      "workspace_path": "agents/agent-alpha",
      "created_at": "2026-01-03T10:00:00Z",
      "last_active": "2026-01-03T12:30:00Z",
      "model": "gpt-4",
      "stats": {
        "messages": 28,
        "files_modified": 8,
        "tasks_completed": 3
      }
    },
    {
      "id": "agent-beta",
      "status": "idle",
      "workspace_path": "agents/agent-beta",
      "created_at": "2026-01-03T10:05:00Z",
      "last_active": "2026-01-03T11:45:00Z",
      "model": "claude-3-opus",
      "stats": {
        "messages": 35,
        "files_modified": 12,
        "tasks_completed": 5
      }
    },
    {
      "id": "agent-gamma",
      "status": "active",
      "workspace_path": "agents/agent-gamma",
      "created_at": "2026-01-03T10:10:00Z",
      "last_active": "2026-01-03T12:28:00Z",
      "model": "deepseek-coder",
      "stats": {
        "messages": 31,
        "files_modified": 10,
        "tasks_completed": 4
      }
    }
  ],
  
  "source_repository": "/path/to/source/repo",
  
  "comparison": {
    "last_run": "2026-01-03T12:00:00Z",
    "metrics_available": true
  }
}
```

### Per-Agent `state.json` (Multi-Agent Mode)

Located at `agents/<agent-id>/.cronus/state.json`:

```json
{
  "agent_id": "agent-alpha",
  "status": "active",
  "created_at": "2026-01-03T10:00:00Z",
  "last_active": "2026-01-03T12:30:00Z",
  
  "repository": {
    "root": ".",
    "branch": "main",
    "last_commit": "abc123def456",
    "uncommitted_changes": true,
    "files_tracked": 156
  },
  
  "session": {
    "id": "session-alpha-12",
    "started_at": "2026-01-03T10:00:00Z",
    "messages": 28,
    "files_modified": 8,
    "tasks_completed": 3,
    "current_task": "Implement caching layer"
  },
  
  "statistics": {
    "total_messages": 142,
    "total_files_modified": 28,
    "total_tasks_completed": 12,
    "uptime_hours": 3.5,
    "average_response_time_ms": 1250
  },
  
  "performance": {
    "tests_passed": 45,
    "tests_failed": 2,
    "code_quality_score": 8.5,
    "last_benchmark": "2026-01-03T12:00:00Z"
  }
}
```

---

## Environment Variables

Cronus supports environment variables for sensitive configuration:

```bash
# API Keys
export OPENAI_API_KEY="sk-..."
export ANTHROPIC_API_KEY="sk-ant-..."
export DEEPSEEK_API_KEY="..."

# Server Configuration
export CRONUS_HOST="localhost"
export CRONUS_PORT="9000"
export CRONUS_LOG_LEVEL="info"

# Model Configuration
export CRONUS_DEFAULT_MODEL="gpt-4"
export CRONUS_DEFAULT_TEMPERATURE="0.7"

# Paths
export CRONUS_CONFIG_DIR="$HOME/.config/cronus"
export CRONUS_CACHE_DIR="$HOME/.cache/cronus"

# Multi-Agent Configuration
export CRONUS_MAX_AGENTS="10"
export CRONUS_AGENT_TIMEOUT="300"
```

---

## CLI Commands

### Initialization

```bash
# Single-agent mode (in existing repository)
cronus init

# Multi-agent mode (in new/empty directory)
cronus init --multi-agent

# Initialize with custom config
cronus init --config=custom-config.json
```

### Agent Management (Multi-Agent)

```bash
# Create agent
cronus agent create <agent-id> [OPTIONS]
  --source=<path>           # Source repository to clone
  --model=<model-name>      # Model to use
  --temperature=<value>     # Temperature setting
  --config=<file>           # Custom config file

# List agents
cronus agent list

# Remove agent
cronus agent remove <agent-id>

# Agent status
cronus agent status <agent-id>
```

### Configuration

```bash
# View configuration
cronus config show

# Edit configuration
cronus config edit

# Set configuration value
cronus config set <key> <value>
  cronus config set agent.model.temperature 0.8

# Validate configuration
cronus config validate
```

### State Management

```bash
# View state
cronus state show

# Reset state
cronus state reset [--confirm]

# Export state
cronus state export > state-backup.json

# Import state
cronus state import < state-backup.json
```

---

## Best Practices

### Single-Agent Mode

1. **Commit `.cronus/config.json` to version control** (without API keys)
2. **Add `.cronus/cache/`, `.cronus/logs/` to `.gitignore`**
3. **Use environment variables for API keys**
4. **Regularly backup `.cronus/context/` for important sessions**
5. **Review `.cronus/state.json` after crashes**

### Multi-Agent Mode

1. **Keep `.cronus/` directory out of version control entirely**
2. **Use shared templates in `.cronus/shared/templates/`**
3. **Monitor disk space** (each agent has a full repository copy)
4. **Set `max_concurrent_agents` to reasonable value**
5. **Regularly clean up unused agents**
6. **Use comparison reports to evaluate agent performance**

### Security

1. **Never commit API keys to version control**
2. **Use environment variables or secret managers**
3. **Restrict file permissions on `.cronus/` directory**
   ```bash
   chmod 700 .cronus
   ```
4. **Regularly rotate API keys**
5. **Review logs for unauthorized access attempts**
