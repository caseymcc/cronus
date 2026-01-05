# .cronus/ Directory Structure Implementation

## Summary

Implemented automatic mode detection and `.cronus/` directory structure management for Cronus, enabling seamless operation in both single-agent and multi-agent modes.

## What Was Implemented

### Core Components

1. **ModeDetector** (`server/cronus/modeDetector.h/cpp`)
   - Automatic detection of operational mode based on directory context
   - Checks for version control, existing config, empty directories
   - Returns detailed detection results with reasoning

2. **DirectoryInitializer** (`server/cronus/directoryInitializer.h/cpp`)
   - Creates appropriate `.cronus/` directory structures
   - Generates default configuration files (config.json, state.json)
   - Supports both single-agent and multi-agent layouts
   - Sets secure permissions (0700) on sensitive directories

3. **Config Integration** (`server/cronus/config.h/cpp`)
   - Auto-detects mode on startup
   - Initializes directory structure if missing
   - Loads JSON and YAML configurations
   - Supports environment variable expansion
   - Tracks operational mode

4. **CLI Enhancements** (`server/cronus/main.cpp`)
   - `--detect-mode`: Show detected mode without making changes
   - `--init`: Initialize directory structure
   - `--mode <mode>`: Force specific operational mode

### Directory Structures

#### Single-Agent Mode
```
.cronus/
├── config.json       # Configuration
├── state.json        # Agent state
├── lock              # Lock file
├── tasks/            # Task queue
├── context/          # Context cache
├── cache/            # Loreforge cache
└── logs/             # Logs
```

#### Multi-Agent Mode
```
.cronus/
├── multi-agent.marker  # Mode marker
├── config.json         # Global config
├── state.json          # Global state
├── shared/
│   ├── cache/
│   ├── templates/
│   └── models/
├── comparison/
└── logs/
```

## Detection Algorithm

1. Check for `.cronus/multi-agent.marker` → Multi-Agent
2. Check for `.cronus/config.json` → Read mode from config
3. Check for `.git/`, `.svn/`, `.hg/` → Single-Agent
4. Check if directory empty/nearly empty → Multi-Agent
5. Default: Directory with content → Single-Agent

## Usage Examples

### Detect Mode
```bash
cronus --detect-mode
```

### Initialize
```bash
# Auto-detect and initialize
cronus --init

# Force specific mode
cronus --init --mode single-agent
cronus --init --mode multi-agent
```

### Run with Mode Override
```bash
cronus --mode multi-agent
```

## Configuration Files

### Single-Agent config.json
- Model configuration (provider, name, temperature)
- API keys with environment variable support
- Loreforge settings
- Workspace configuration

### Multi-Agent config.json
- Server configuration (host, port, CORS)
- Workspace settings (source repo, sync)
- Agent list (initially empty)
- Shared resource configuration
- Comparison settings

### state.json
- Tracks operational state
- Single-agent: agent status, stats, current task
- Multi-agent: workspace info, agent list

## Testing Results

All tests successful:

1. ✅ Mode detection in existing Git repository → Single-Agent
2. ✅ Mode detection in empty directory → Multi-Agent
3. ✅ Single-agent initialization creates correct structure
4. ✅ Multi-agent initialization creates correct structure
5. ✅ Forced mode override works correctly
6. ✅ Generated config files are valid JSON
7. ✅ Directory permissions set correctly (0700)

## Files Modified/Created

### New Files
- `server/cronus/modeDetector.h`
- `server/cronus/modeDetector.cpp`
- `server/cronus/directoryInitializer.h`
- `server/cronus/directoryInitializer.cpp`
- `docs/MODE-DETECTION.md`

### Modified Files
- `server/cronus/config.h` - Added mode tracking and JSON config support
- `server/cronus/config.cpp` - Integrated mode detection and initialization
- `server/cronus/main.cpp` - Added CLI options for mode control
- `server/cronus/CMakeLists.txt` - Added new source files

## Next Steps

Future enhancements (not yet implemented):

1. **Agent Management** (Multi-Agent Mode)
   - `cronus agent create <id> --source=<repo>`
   - `cronus agent list`
   - `cronus agent remove <id>`
   - Repository cloning for agents

2. **Multi-Agent Server**
   - REST API for agent management
   - Parallel agent execution
   - Agent status monitoring
   - Comparison metrics

3. **Migration Tools**
   - Convert between single and multi-agent modes
   - Backup/restore configurations
   - Config file validation

4. **Enhanced Detection**
   - Project type detection (Python, JS, etc.)
   - Automatic model selection based on project
   - Workspace complexity analysis

## Documentation

See `docs/MODE-DETECTION.md` for complete documentation including:
- Detailed usage guide
- API reference
- Configuration examples
- Security considerations
- Migration guide

## Build Instructions

Built successfully with:
```bash
./run_local.sh ninja -C build/linux_x64_debug
```

All existing tests pass. No breaking changes to existing functionality.
