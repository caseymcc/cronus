# Mode Detection and Directory Management

This document describes Cronus's automatic mode detection and `.cronus/` directory structure management system.

## Overview

Cronus operates in two modes:

- **Single-Agent Mode**: For working within an existing repository with a single AI agent
- **Multi-Agent Mode**: For running multiple agents in parallel, each with isolated workspaces

Cronus automatically detects the appropriate mode based on the current directory context and initializes the necessary directory structure.

## Mode Detection Algorithm

When Cronus starts, it follows this detection algorithm:

1. **Check for multi-agent marker**: If `.cronus/multi-agent.marker` exists → Multi-Agent Mode
2. **Check for existing config**: If `.cronus/config.json` exists, read the `mode` field
3. **Check for version control**: If `.git/`, `.svn/`, or `.hg/` exists → Single-Agent Mode
4. **Check if directory is empty**: If directory is empty or nearly empty → Multi-Agent Mode
5. **Default**: Directory with content but no VCS → Single-Agent Mode

## Command-Line Usage

### Detect Mode

To see what mode would be detected without making any changes:

```bash
cronus --detect-mode
```

Output example:
```
Mode Detection Results:
  Working Directory: /path/to/project
  Detected Mode: single-agent
  Reason: Version control repository detected (.git, .svn, or .hg)
  Has Existing Config: No
  Has Version Control: Yes
  Is Empty Directory: No
  Has Multi-Agent Marker: No
```

### Initialize Directory Structure

To initialize the `.cronus/` directory structure:

```bash
# Auto-detect mode and initialize
cronus --init

# Force specific mode
cronus --init --mode single-agent
cronus --init --mode multi-agent
```

### Override Mode at Runtime

You can force a specific mode when running Cronus:

```bash
cronus --mode multi-agent
```

## Directory Structures

### Single-Agent Mode

Created in existing repositories or directories with version control:

```
.cronus/
├── config.json       # Configuration (model, API keys, etc.)
├── state.json        # Agent state and statistics
├── lock              # Lock file for concurrent access
├── tasks/            # Task queue and history
├── context/          # Context cache
├── cache/            # Loreforge embeddings and other cache
└── logs/             # Log files
```

**Example `config.json`:**
```json
{
  "version": "1.0",
  "mode": "single-agent",
  "model": {
    "provider": "openai",
    "name": "gpt-3.5-turbo",
    "temperature": 0.7,
    "max_tokens": 4096
  },
  "api_keys": {
    "openai": "${OPENAI_API_KEY}",
    "anthropic": "${ANTHROPIC_API_KEY}"
  },
  "loreforge": {
    "enabled": true,
    "cache_dir": ".cronus/cache"
  },
  "workspace": {
    "working_directory": "."
  }
}
```

**Example `state.json`:**
```json
{
  "version": "1.0",
  "mode": "single-agent",
  "created_at": "2026-01-04T12:00:00Z",
  "last_updated": "2026-01-04T12:00:00Z",
  "agent": {
    "status": "idle",
    "current_task": null,
    "stats": {
      "messages": 0,
      "files_modified": 0,
      "tasks_completed": 0
    }
  }
}
```

### Multi-Agent Mode

Created in empty directories or when explicitly requested:

```
.cronus/
├── multi-agent.marker  # Marker file indicating multi-agent mode
├── config.json         # Global server configuration
├── state.json          # Global state tracking all agents
├── shared/             # Resources shared across agents
│   ├── cache/          # Shared embeddings and cache
│   ├── templates/      # Shared prompt templates
│   └── models/         # Model configurations
├── comparison/         # Agent comparison data
└── logs/               # Server logs

agents/                 # Created when agents are added (not auto-created)
├── agent-1/
│   ├── .cronus/        # Agent-specific configuration
│   ├── .git/           # Agent's repository (if cloned)
│   └── ...             # Agent's workspace files
└── agent-2/
    └── ...
```

**Example `config.json`:**
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
    "source_repository": "",
    "auto_sync": false,
    "sync_interval_minutes": 30
  },
  "agents": [],
  "shared": {
    "cache_dir": ".cronus/shared/cache",
    "templates_dir": ".cronus/shared/templates",
    "models_dir": ".cronus/shared/models",
    "loreforge": {
      "enabled": true,
      "embedding_model": "text-embedding-3-small"
    }
  },
  "comparison": {
    "enabled": true,
    "metrics": ["correctness", "code_quality", "performance"]
  }
}
```

**Example `state.json`:**
```json
{
  "version": "1.0",
  "mode": "multi-agent",
  "created_at": "2026-01-04T12:00:00Z",
  "last_updated": "2026-01-04T12:00:00Z",
  "workspace": {
    "root": ".",
    "agent_count": 0
  },
  "agents": []
}
```

## Configuration Priority

Cronus loads configuration in this order (later overrides earlier):

1. Environment variables (`CRONUS_MODEL`, `OPENAI_API_KEY`, etc.)
2. User-global config (`~/.cronus/config.yml` or `~/.cronus/config.json`)
3. Local config (`./.cronus/config.yml` or `./.cronus/config.json`)
4. Command-line arguments

## Environment Variables

Configuration files support environment variable expansion:

```json
{
  "api_keys": {
    "openai": "${OPENAI_API_KEY}",
    "anthropic": "${ANTHROPIC_API_KEY}"
  }
}
```

Variables in the format `${VAR_NAME}` are automatically expanded when the configuration is loaded.

## API Access

The mode detection and initialization functionality is available programmatically:

### C++ API

```cpp
#include "cronus/modeDetector.h"
#include "cronus/directoryInitializer.h"

// Detect mode
auto result = cronus::ModeDetector::detect(workingDirectory);
std::cout << "Mode: " << cronus::operationalModeToString(result.mode) << "\n";

// Initialize directory structure
auto initResult = cronus::DirectoryInitializer::initialize(
    workingDirectory,
    cronus::OperationalMode::MultiAgent
);

if (initResult.success) {
    std::cout << initResult.message << "\n";
}
```

### Config Access

```cpp
#include "cronus/config.h"

auto& config = cronus::Config::instance();
config.load(resourcePath);

// Get current mode
auto mode = config.getOperationalMode();

// Force mode
config.setOperationalMode(cronus::OperationalMode::MultiAgent);
```

## Security

The `.cronus/` directory is created with restrictive permissions (0700, user-only access) to protect sensitive configuration data like API keys.

## Migration

If you have an existing project without a `.cronus/` directory:

1. **Automatic**: Just run `cronus` - it will auto-detect and initialize
2. **Manual**: Run `cronus --init` to initialize with detected mode
3. **Explicit**: Run `cronus --init --mode <mode>` to force a specific mode

If you have a `.cronus/` directory from an older version, Cronus will:

1. Detect the mode from the existing `config.json` if present
2. Validate the directory structure
3. Warn if structure is incomplete but continue operation

## See Also

- [Architecture Documentation](architecture.md) - Dual-mode system design
- [Configuration Reference](configuration.md) - Detailed configuration options
- [Development Roadmap](development/ROADMAP.md) - Future enhancements
